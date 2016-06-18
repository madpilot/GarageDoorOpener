#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

#define OPEN_STATE  1
#define CLOSED_STATE 0
#define OPENING_STATE 2
#define CLOSING_STATE 3

#define STATE_TOPIC "home-assistant/garage"
#define COMMAND_TOPIC "home-assistant/garage/set"

#define OPEN_COMMAND "OPEN"
#define CLOSE_COMMAND "CLOSE"

#define OPENED_PAYLOAD "OPENED"
#define CLOSED_PAYLOAD "CLOSED"
#define OPENING_PAYLOAD "OPENING"
#define CLOSING_PAYLOAD "CLOSING"

#define QOS_LEVEL 0

// For debugging
WiFiServer telnetServer(23);
WiFiClient telnetClient;

void TelnetSetup() {
  telnetServer.begin();
  telnetServer.setNoDelay(true);
}

void TelnetLoop() {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      Serial.println("New client connected");
      telnetClient = telnetServer.available();
      telnetClient.flush();
    }
  } else {
    if(telnetClient && telnetClient.connected()) {
      telnetClient.stop();
    }
  }
}

void TelnetPrintLn(char *message) {
  if(telnetClient && telnetClient.connected()) {
    telnetClient.println(message);
  }
}

// PubSub client
WiFiClient espClient;
PubSubClient pubSubClient(espClient);
void PubSubCallback(char* topic, byte* payload, unsigned int length);
long lastPubSubConnectionAttempt = 0;

void PubSubSetup() {
  pubSubClient.setServer("192.168.1.16", 1883);
  pubSubClient.setCallback(PubSubCallback);
}

boolean PubSubConnect() {
  Serial.print("Connecting to MQTT server...");
  
  if(!pubSubClient.connect("garage")) {
    Serial.println("\nCouldn't connect to MQTT server. Will try again in 5 seconds.");
    return false;
  }
  
  if(!pubSubClient.subscribe(COMMAND_TOPIC, QOS_LEVEL)) {
    Serial.print("\nUnable to subscribe to ");
    Serial.println(COMMAND_TOPIC);
    pubSubClient.disconnect();
    return false;
  }

  Serial.println(" Connected.");
  return true;
}

void PubSubLoop() {
  if(!pubSubClient.connected()) {
    long now = millis();

    if(now - lastPubSubConnectionAttempt > 5000) {
      lastPubSubConnectionAttempt = now;
      if(PubSubConnect()) {
        lastPubSubConnectionAttempt = 0;
      }
    }
  } else {
    pubSubClient.loop();
  }
}

void closeDoor();
void openDoor();

void PubSubCallback(char* topic, byte* payload, unsigned int length) {
  char *p = (char *)malloc((length + 1) * sizeof(char *));
  strncpy(p, (char *)payload, length);
  p[length] = '\0';

  Serial.print("Callback called: ");
  Serial.print(topic);
  Serial.print(" ");
  Serial.println(p);
  
  if(strcmp(p, CLOSE_COMMAND) == 0) {
    Serial.println("Closing the garage");
    closeDoor();
  } else if(strcmp(p, OPEN_COMMAND) == 0) {
    Serial.println("Opening the garage");
    openDoor();
  }
}

void connectWifi(const char* ssid, const char* password) {
  int WiFiCounter = 0;
  // We start by connecting to a WiFi network
  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && WiFiCounter < 30) {
    delay(1000);
    WiFiCounter++;
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("garage")) {
    Serial.println("Couldn't set mDNS name");
    return;
  }

  Serial.println("Device is available at garage.local");
}


int doorState = CLOSED;

int getDoorState() {
  return doorState;
}

void setDoorState(int state) {
  switch(state) {
    case OPENING_STATE:
      pubSubClient.publish(STATE_TOPIC, OPENING_PAYLOAD);
      break;
    case CLOSING_STATE:
      pubSubClient.publish(STATE_TOPIC, CLOSING_PAYLOAD);
      break;
    case OPEN_STATE:
      pubSubClient.publish(STATE_TOPIC, OPENED_PAYLOAD);
      break;
    case CLOSED_STATE:
      pubSubClient.publish(STATE_TOPIC, CLOSED_PAYLOAD);
      break; 
  }
  doorState = state;
}

void openDoor() {
  int current = getDoorState();
  if(current != OPEN_STATE && current != OPENING_STATE) {
    setDoorState(OPEN_STATE);
  }
}

void closeDoor() {
  int current = getDoorState();
  if(current != CLOSED_STATE && current != CLOSING_STATE) {
    setDoorState(CLOSED_STATE);
  }
}

void setup() {
  Serial.begin(9600);
  connectWifi("ssid", "password");
  TelnetSetup();
  PubSubSetup();

  pubSubClient.publish(STATE_TOPIC, CLOSED_PAYLOAD);
}

void loop() {
  TelnetLoop();
  PubSubLoop();
}
