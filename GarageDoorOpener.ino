#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#include "WiFiManager.h" 
#include "Config.h"
#include "MQTT.h"

#define DEFAULT_SSID "garage"

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

void setNetworkName(const char *name) {
  wifi_station_set_hostname((char *)name);
  
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

bool configMode = false;
Config config;

void configSetup() {
  config.addKey("deviceName", DEFAULT_SSID, 63);
  
  config.addKey("mqttServer", "", 255);
  config.addKey("mqttPort", "8123", 6);
  config.addKey("mqttUsername", "", 31);
  config.addKey("mqttPassword", "", 31);
  
  config.addKey("cert", "", 1024);
  config.addKey("certKey", "", 1024);
  
  config.read();
}

int saveFlag = false;
void saveCallback() {
  saveFlag = true;
}

void wifiSetup() {
  WiFiManager wifiManager;

  // Need an iterator
  ConfigOption *deviceName = config.get("deviceName");
  WiFiManagerParameter deviceName_parameter(deviceName->getKey(), deviceName->getValue(), deviceName->getLength());
  wifiManager.addParameter(&deviceName_parameter);

  ConfigOption *mqttServer = config.get("mqttServer");
  WiFiManagerParameter mqttServer_parameter(mqttServer->getKey(), mqttServer->getValue(), mqttServer->getLength());
  wifiManager.addParameter(&mqttServer_parameter);

  ConfigOption *mqttPort = config.get("mqttPort");
  WiFiManagerParameter mqttPort_parameter(mqttPort->getKey(), mqttPort->getValue(), mqttPort->getLength());
  wifiManager.addParameter(&mqttPort_parameter);

  ConfigOption *mqttUsername = config.get("mqttUsername");
  WiFiManagerParameter mqttUsername_parameter(mqttUsername->getKey(), mqttUsername->getValue(), mqttUsername->getLength());
  wifiManager.addParameter(&mqttUsername_parameter);

  ConfigOption *mqttPassword = config.get("mqttPassword");
  WiFiManagerParameter mqttPassword_parameter(mqttPassword->getKey(), mqttPassword->getValue(), mqttPassword->getLength());
  wifiManager.addParameter(&mqttPassword_parameter);

  ConfigOption *cert = config.get("cert");
  WiFiManagerParameter cert_parameter(cert->getKey(), cert->getValue(), cert->getLength());
  wifiManager.addParameter(&cert_parameter);

  ConfigOption *certKey = config.get("certKey");
  WiFiManagerParameter certKey_parameter(certKey->getKey(), certKey->getValue(), certKey->getLength());
  wifiManager.addParameter(&certKey_parameter);

  wifiManager.setSaveConfigCallback(saveCallback);
  
  if(configMode) {
    Serial.println("Going in to config mode");
    wifiManager.startConfigPortal(CONFIG_AP_SSID);
  } else {  
    wifiManager.autoConnect(CONFIG_AP_SSID);
  }

  deviceName->setValue(deviceName_parameter.getValue());
  mqttServer->setValue(mqttServer_parameter.getValue());
  mqttPort->setValue(mqttPort_parameter.getValue());
  mqttUsername->setValue(mqttUsername_parameter.getValue());
  mqttPassword->setValue(mqttPassword_parameter.getValue());
  cert->setValue(cert_parameter.getValue());
  certKey->setValue(certKey_parameter.getValue());
  
  if(saveFlag) {
    config.write();
  }
    
  setNetworkName(deviceName->getValue());
}

void setup() {
  Serial.begin(115200);
  configSetup();
  wifiSetup();
  
  PubSubSetup(&pubSubClient, PubSubCallback);
  pubSubClient.publish(STATE_TOPIC, CLOSED_PAYLOAD);
}

void loop() {
  if(configMode) {
    return;
  }
  
  PubSubLoop(&pubSubClient);
}
