#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <FS.h>
#include "WiFiManager.h" 

#include <PubSubClient.h>
#include "MQTT.h"

#define OPEN_STATE  1
#define CLOSED_STATE 0
#define OPENING_STATE 2
#define CLOSING_STATE 3

// PubSub client
WiFiClient espClient;
PubSubClient pubSubClient(espClient);

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

// WIFI functions
#define HOSTNAME "garage"
#define CONFIG_AP_SSID "garage"

const byte DNS_PORT = 53;
DNSServer dnsServer;

ESP8266WebServer webServer(80);

/*
IPAddress configIP(192, 168, 4, 1);
IPAddress configNetmask(255, 255, 255, 0);

void WifiSetup() {
  Serial.println("Setting up WIFI configuration Network");
  WiFi.softAPConfig(configIP, configIP, configNetmask);
  WiFi.softAP(HOSTNAME);
  delay(500);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", configIP);
}
*/


void getIndex() {
  File f = SPIFFS.open("/index.html", "r");
  webServer.setContentLength(f.size());
  webServer.streamFile(f, "text/html");

  f.close();
}

void getBrowseJSON() {
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  Serial.println("Scanning for WIFI access points");

    int n = WiFi.scanNetworks();
  webServer.sendContent("[");
  for (int i = 0; i < n; i++) {
    webServer.sendContent("{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + WiFi.RSSI(i) + ",\"encryption\":" + String(WiFi.encryptionType(i)) + "}");
    if(i != n - 1) {
      webServer.sendContent(",");
    }
  }
  webServer.sendContent("]");

  Serial.println("JSON response sent");
}

void postSave() {
  Serial.println("Saving Configuration Settings");
  Serial.print("Sent ");
  Serial.println(webServer.arg("ssid"));
  Serial.print("Passkey ");
  Serial.println(webServer.arg("passkey"));

  webServer.send(200, "text/plain", "done");
}

void WebServerSetup() {
  SPIFFS.begin();
  webServer.on("/", getIndex);
  webServer.on("/browse.json", getBrowseJSON);
  webServer.on("/save", postSave);
  webServer.begin();
}

void WebServerLoop() {
  webServer.handleClient();
}

void setNetworkName(const char *name) {
  if (!MDNS.begin(name)) {
    Serial.println("Couldn't set mDNS name");
    return;
  }

  Serial.print("Device is available at ");
  Serial.print(name);
  Serial.println(".local");
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

bool inAutoConfig = false;
bool inManualConfig = true;

void setup() {
  Serial.begin(9600);
  WiFiManager wifiManager;

  if(inManualConfig) {
    wifiManager.startConfigPortal("garage", "door");
    return;
  }
  
  if(wifiManager.autoConnect("garage", "door")) {  
    setNetworkName("garage");
    WebServerSetup();
    PubSubSetup(&pubSubClient, PubSubCallback);
    pubSubClient.publish(STATE_TOPIC, CLOSED_PAYLOAD);
  } else {
    inAutoConfig = true;
  }
}

void loop() {
  if(inAutoConfig || inManualConfig) {
    return;
  }
  
  PubSubLoop(&pubSubClient);
  WebServerLoop();
}
