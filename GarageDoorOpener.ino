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

#define OPEN_COMMAND "OPEN"
#define CLOSE_COMMAND "CLOSE"

#define OPENED_PAYLOAD "OPENED"
#define CLOSED_PAYLOAD "CLOSED"
#define OPENING_PAYLOAD "OPENING"
#define CLOSING_PAYLOAD "CLOSING"

#define PUBLISH_CHANNEL "home-assistant/garage"
#define SUBSCRIBE_CHANNEL "home-assistant/garage/set"

#define RELAY_CLOSE_TIME 100
#define RELAY            1
#define OPENED_SWITCH      2
#define CLOSED_SWITCH    3

int saveFlag = false;
bool configMode = false;
int doorState = CLOSED;

Config config;
PubSub *pubSub = NULL;

void closeDoor();
void openDoor();

void pubSubCallback(char* topic, byte* payload, unsigned int length) {
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

int getDoorState() {
  return doorState;
}

void setDoorState(int state) {
  switch(state) {
    case OPENING_STATE:
      pubSub->publish(OPENING_PAYLOAD);
      break;
    case CLOSING_STATE:
      pubSub->publish(CLOSING_PAYLOAD);
      break;
    case OPEN_STATE:
      pubSub->publish(OPENED_PAYLOAD);
      break;
    case CLOSED_STATE:
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

void readDoorLoop() {
  int current = getDoorState();
  int opened = digitalRead(OPENED_SWITCH);
  int closed = digitalRead(CLOSED_SWITCH);

  if(opened && !closed) {
    // Fully opened
    if(current != OPEN_STATE) {
      setDoorState(OPEN_STATE);
    }
  } else if(!opened && closed) {
    // Fully closed
    if(current != CLOSED_STATE) {
      setDoorState(CLOSED_STATE);
    }
  } else if(!opened && !closed) {
    // Either opening or closing
    if(current == CLOSED) {
      setDoorState(CLOSING_STATE);
    } else if(current == OPEN_STATE) {
      setDoorState(OPENING_STATE);
    }
  }
}

void openDoor() {
  int current = getDoorState();
  if(current != OPEN_STATE && current != OPENING_STATE) {
    trigger();

    // If the door wasn't fully open, switch the state, as we have no sensor that can do that.
    if(current == OPENING_STATE) {
      setDoorState(CLOSING_STATE);
    }
  }
}

void closeDoor() {
  int current = getDoorState();
  if(current != CLOSED_STATE && current != CLOSING_STATE) {
    trigger();

    // If the door wasn't fully closed, switch the state, as we have no sensor that can do that.
    if(current == CLOSING_STATE) {
      setDoorState(OPENING_STATE);
    }
  }
}

void configSetup() {
  config.addKey("deviceName", DEFAULT_SSID, 63);
  
  config.addKey("mqttServer", "", 255);
  config.addKey("mqttPort", "1883", 6);
  config.addKey("mqttUsername", "", 31);
  config.addKey("mqttPassword", "", 31);
  config.addKey("mqttSubscribeChannel", SUBSCRIBE_CHANNEL, 31);
  config.addKey("mqttPublishChannel", PUBLISH_CHANNEL, 31);
  
  config.addKey("cert", "", 1024);
  config.addKey("certKey", "", 1024);
  
  config.read();
}

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

  ConfigOption *mqttPublishChannel = config.get("mqttPublishChannel");
  WiFiManagerParameter mqttPublishChannel_parameter(mqttPublishChannel->getKey(), mqttPublishChannel->getValue(), mqttPublishChannel->getLength());
  wifiManager.addParameter(&mqttPublishChannel_parameter);

  ConfigOption *mqttSubscribeChannel = config.get("mqttSubscribeChannel");
  WiFiManagerParameter mqttSubscribeChannel_parameter(mqttSubscribeChannel->getKey(), mqttSubscribeChannel->getValue(), mqttSubscribeChannel->getLength());
  wifiManager.addParameter(&mqttSubscribeChannel_parameter);

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
  mqttPublishChannel->setValue(mqttPublishChannel_parameter.getValue());
  mqttSubscribeChannel->setValue(mqttSubscribeChannel_parameter.getValue());
  
  cert->setValue(cert_parameter.getValue());
  certKey->setValue(certKey_parameter.getValue());
  
  if(saveFlag) {
    config.write();
  }
    
  setNetworkName(deviceName->getValue());
}

void pubSubSetup() {
  pubSub = new PubSub(config.get("mqttServer")->getValue(), atoi(config.get("mqttPort")->getValue()), config.get("deviceName")->getValue());
  
  pubSub->setCallback(pubSubCallback);
  pubSub->setSubscribeChannel(config.get("mqttSubscribeChannel")->getValue());
  pubSub->setPublishChannel(config.get("mqttPublishChannel")->getValue());
  pubSub->setAuthentication(config.get("mqttUsername")->getValue(), config.get("mqttPassword")->getValue());
  pubSub->setCertificate(config.get("cert")->getValue(), config.get("certKey")->getValue());
}


void setup() {
  Serial.begin(115200);
  configSetup();
  wifiSetup();
  pubSubSetup();
}

void loop() {
  if(configMode) {
    return;
  }
  
  pubSub->loop();
  triggerLoop();
  readDoorLoop();
}
