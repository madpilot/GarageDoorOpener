#include <WiFiUdp.h>

#include "Config.h"
#include "MQTT.h"
#include "WifiManager.h"

#include "Syslog.h"
Syslog Syslogger;

#define CONFIG_AP_SSID "garage"

#define OPEN_STATE  1
#define CLOSED_STATE 0

#define OPEN_COMMAND "OPEN"
#define CLOSE_COMMAND "CLOSE"
#define STOP_COMMAND "STOP"

#define OPENED_PAYLOAD "OPENED"
#define CLOSED_PAYLOAD "CLOSED"
#define OPENING_PAYLOAD "OPENING"
#define CLOSING_PAYLOAD "CLOSING"

#define TLS_NO "0"
#define TLS_YES "1"

#define RELAY_CLOSE_TIME 500
#define RELAY_GND        0
#define RELAY            2
#define OPENED_SWITCH    1
#define CLOSED_SWITCH    3

int saveFlag = false;
bool configMode = false;
int doorState = CLOSED_STATE;

Config config;
PubSub *pubSub = NULL;

void closeDoor();
void openDoor();

WifiManager wifiManager(&config);

void pubSubCallback(char* topic, byte* payload, unsigned int length) {
  char *p = (char *)malloc((length + 1) * sizeof(char *));
  if(p == NULL) {
    Syslogger.send(SYSLOG_WARNING, "Unable to read MQTT message: Payload too large.");
    return;
  }
  strncpy(p, (char *)payload, length);
  p[length] = '\0';

  Syslogger.send(SYSLOG_DEBUG, "Command Received:");
  Syslogger.send(SYSLOG_DEBUG, p);
  
  if(strcmp(p, CLOSE_COMMAND) == 0) {
    Syslogger.send(SYSLOG_INFO, "Closing the garage door");
    closeDoor();
  } else if(strcmp(p, OPEN_COMMAND) == 0) {
    Syslogger.send(SYSLOG_INFO, "Opening the garage door");
    openDoor();
  } else if(strcmp(p, STOP_COMMAND) == 0) {
    Syslogger.send(SYSLOG_INFO, "Stopping the garage door");
    stopDoor();
  }
  free(p);
}

int getDoorState() {
  return doorState;
}

void setDoorState(int state) {
  switch(state) {
    case OPEN_STATE:
      Syslogger.send(SYSLOG_INFO, "Garage Door is Open");
      pubSub->publish(OPENED_PAYLOAD);
      break;
    case CLOSED_STATE:
      Syslogger.send(SYSLOG_INFO, "Garage Door is Closed");
      pubSub->publish(CLOSED_PAYLOAD);
      break;
  }
  doorState = state;
}

// Close the relay for RELAY_CLOSE_TIME
long triggerStart = 0;
void triggerLoop() {
  if(triggerStart > 0) {
    long now = millis();

    if(now - triggerStart > RELAY_CLOSE_TIME) {
      digitalWrite(RELAY, LOW);
      triggerStart = 0;
    }
  }
}

void trigger() {
  digitalWrite(RELAY, HIGH);
  triggerStart = millis();
}

void notifyInitialDoorState() {
  int closed = !digitalRead(CLOSED_SWITCH);

  if(closed) {
    setDoorState(CLOSED_STATE);
  } else {
    setDoorState(OPEN_STATE);
  }
}

void readDoorLoop() {
  int current = getDoorState();
  // Inverted because we pull them low when switches are closed
  int closed = !digitalRead(CLOSED_SWITCH);
  
  if(closed) {
    // Fully opened
    if(current != CLOSED_STATE) {
      setDoorState(CLOSED_STATE);
    }
  } else {
    if(current != OPEN_STATE) {
      setDoorState(OPEN_STATE);
    }
  }
}

void openDoor() {
  int current = getDoorState();
  if(current != OPEN_STATE) {
    trigger();
  }
}

void closeDoor() {
  int current = getDoorState();
  if(current != CLOSED_STATE) {
    trigger();
  }
}

void stopDoor() {
  trigger();
}

config_result configSetup() {
  config_result result = config.read();
  switch(result) {
    case E_CONFIG_OK:
      Serial.println("Config read");
      break;
    case E_CONFIG_FS_ACCESS:
      Serial.println("E_CONFIG_FS_ACCESS: Couldn't access file system");
      break;
    case E_CONFIG_FILE_NOT_FOUND:
      Serial.println("E_CONFIG_FILE_NOT_FOUND: File not found");
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println("E_CONFIG_FILE_OPEN: Couldn't open file");
      break;
    case E_CONFIG_PARSE_ERROR:
      Serial.println("E_CONFIG_PARSE_ERROR: File was not parsable");
      break;
  }
  return result;
}

void wifiSetup() {
  while(wifiManager.loop() != E_WIFI_OK) {
    Serial.println("Could not connect to WiFi. Will try again in 5 seconds");
    delay(5000);
  }
  Serial.println("Connected to WiFi");
}

WiFiUDP syslogSocket;
void syslogSetup() {
  if(config.get_syslog()) {
    Serial.println("Syslog enabled");
    Syslogger = Syslog(syslogSocket, config.get_syslogHost(), config.get_syslogPort(), config.get_deviceName(), config.get_deviceName());
    Syslogger.setMinimumSeverity(config.get_syslogLevel());
    Syslogger.send(SYSLOG_INFO, "Device booted.");
  }
}

void pubSubSetup() {  
  pubSub = new PubSub(config.get_mqttServerName(), config.get_mqttPort(), config.get_mqttTLS(), config.get_deviceName());
  pubSub->setCallback(pubSubCallback);
  
  pubSub->setSubscribeChannel(config.get_mqttSubscribeChannel());
  pubSub->setPublishChannel(config.get_mqttPublishChannel());

  pubSub->setAuthMode(config.get_mqttAuthMode());

  Syslogger.send(SYSLOG_DEBUG, "Loading certificate.");
  pubSub->setFingerprint(config.get_mqttFingerprint());
  
  switch(pubSub->loadCertificate("/client.crt.der")) {
    case E_MQTT_OK:
      Syslogger.send(SYSLOG_DEBUG, "Certificate loaded.");
      break;
    case E_MQTT_CERT_NOT_LOADED:
      Syslogger.send(SYSLOG_ERROR, "Certificate not loaded.");
      break;
    case E_MQTT_CERT_FILE_NOT_FOUND:
      Syslogger.send(SYSLOG_ERROR, "Couldn't find certificate file.");
      break;
    case E_MQTT_SPIFFS:
      Syslogger.send(SYSLOG_CRITICAL, "Unable to start SPIFFS.");
      break;
  }
  
  Syslogger.send(SYSLOG_DEBUG, "Loading private key.");    
  switch(pubSub->loadPrivateKey("/client.key.der")) {
     case E_MQTT_OK:
      Syslogger.send(SYSLOG_DEBUG, "Private key loaded.");
      break;
    case E_MQTT_PRIV_KEY_NOT_LOADED:
      Syslogger.send(SYSLOG_ERROR, "Private key not loaded.");
      break;
    case E_MQTT_PRIV_KEY_FILE_NOT_FOUND:
      Syslogger.send(SYSLOG_ERROR, "Couldn't find private key file.");
      break;
    case E_MQTT_SPIFFS:
      Syslogger.send(SYSLOG_CRITICAL, "Unable to start SPIFFS.");
      break;
  }
}

void setup() {
  // Serial.begin(115200);

  pinMode(RELAY_GND, OUTPUT);
  digitalWrite(RELAY_GND, HIGH);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  
  pinMode(CLOSED_SWITCH, INPUT);
 
  
  digitalWrite(RELAY, LOW);
  digitalWrite(RELAY_GND, LOW);
  
  config_result configResult = configSetup();
  if(configResult != E_CONFIG_OK) {
    configMode = true;
    return;
  }
  
  wifiSetup();
  syslogSetup();
  pubSubSetup();
  Syslogger.send(SYSLOG_DEBUG, "Ready for commands");
}

bool firstRun = true;
void loop() {
  if(configMode) {
    Syslogger.send(SYSLOG_DEBUG, "Config mode");
    return;
  }

  if(wifiManager.loop() == E_WIFI_OK && pubSub->loop() == E_MQTT_OK) {
    if(firstRun) {
      notifyInitialDoorState();
      firstRun = false;
      Syslogger.send(SYSLOG_DEBUG, "MQTT Server initialised with initial state");
    }
    readDoorLoop();
    triggerLoop();
  }
}
