#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//#include "WiFiManager.h" 
#include "Config.h"
#include "MQTT.h"

#include "Syslogger.h"
Syslog *Syslogger;

#define CONFIG_AP_SSID "garage"

#define OPEN_STATE  1
#define CLOSED_STATE 0
#define OPENING_STATE 2
#define CLOSING_STATE 3

#define OPEN_COMMAND "OPEN"
#define CLOSE_COMMAND "CLOSE"
#define STOP_COMMAND "STOP"

#define OPENED_PAYLOAD "OPENED"
#define CLOSED_PAYLOAD "CLOSED"
#define OPENING_PAYLOAD "OPENING"
#define CLOSING_PAYLOAD "CLOSING"

#define PUBLISH_CHANNEL "home-assistant/cover"
#define SUBSCRIBE_CHANNEL "home-assistant/cover/set"

#define TLS_NO "0"
#define TLS_YES "1"

#define RELAY_CLOSE_TIME 100
#define RELAY_GND        0
#define RELAY            2
#define OPENED_SWITCH    1
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

  if(strcmp(p, CLOSE_COMMAND) == 0) {
    Syslogger->send(SYSLOG_INFO, "Closing the garage door");
    closeDoor();
  } else if(strcmp(p, OPEN_COMMAND) == 0) {
    Syslogger->send(SYSLOG_INFO, "Opening the garage door");
    openDoor();
  } else if(strcmp(p, STOP_COMMAND) == 0) {
    Syslogger->send(SYSLOG_INFO, "Stopping the garage door");
    stopDoor();
  }
  free(p);
}

void setNetworkName(const char *name) {
  //wifi_station_set_hostname((char *)name);
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
  // Inverted because we pull them low when switches are closed
  int opened = !digitalRead(OPENED_SWITCH);
  int closed = !digitalRead(CLOSED_SWITCH);
  
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
    if(current == OPEN_STATE) {
      setDoorState(CLOSING_STATE);
    } else if(current == CLOSED_STATE) {
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

void stopDoor() {
  int current = getDoorState();

  // Triggering again, stops the door. If we are already stopped
  // Don't do anything.
  if(current != CLOSED_STATE && current != OPEN_STATE) {
    trigger();
    setDoorState(OPEN_STATE);  
  }
}

void configSetup() {
  Serial.println("configSetup();");
  
  Serial.println(ESP.getFreeHeap());
  
  switch(config.read()) {
    case E_CONFIG_OK:
      Serial.println("Config read");
      return;
    case E_CONFIG_FS_ACCESS:
      Serial.println("E_CONFIG_FS_ACCESS: Couldn't access file system");
      return;
    case E_CONFIG_FILE_NOT_FOUND:
      Serial.println("E_CONFIG_FILE_NOT_FOUND: File not found");
      return;
    case E_CONFIG_FILE_OPEN:
      Serial.println("E_CONFIG_FILE_OPEN: Couldn't open file");
      return;
    case E_CONFIG_PARSE_ERROR:
      Serial.println("E_CONFIG_PARSE_ERROR: File was not parsable");
      return;
  }
  Serial.println(ESP.getFreeHeap());
  
}

void saveCallback() {
  saveFlag = true;
}

void wifiSetup() {
  //WiFiManager wifiManager;
  /*
  ConfigOption *ssid = config.get("ssid");
  WiFiManagerParameter ssid_parameter(ssid->getKey(), ssid->getValue(), ssid->getLength());
  wifiManager.addParameter(&ssid_parameter);

  ConfigOption *passkey = config.get("passkey");
  WiFiManagerParameter passkey_parameter(passkey->getKey(), passkey->getValue(), passkey->getLength());
  wifiManager.addParameter(&passkey_parameter);

  ConfigOption *encryption = config.get("encryption");
  WiFiManagerParameter encryption_parameter(encryption->getKey(), encryption->getValue(), encryption->getLength());
  wifiManager.addParameter(&encryption_parameter);
  
  ConfigOption *mqttDeviceName = config.get("mqttDeviceName");
  WiFiManagerParameter mqttDeviceName_parameter(mqttDeviceName->getKey(), mqttDeviceName->getValue(), mqttDeviceName->getLength());
  wifiManager.addParameter(&mqttDeviceName_parameter);

  ConfigOption *mqttServer = config.get("mqttServer");
  WiFiManagerParameter mqttServer_parameter(mqttServer->getKey(), mqttServer->getValue(), mqttServer->getLength());
  wifiManager.addParameter(&mqttServer_parameter);

  ConfigOption *mqttPort = config.get("mqttPort");
  WiFiManagerParameter mqttPort_parameter(mqttPort->getKey(), mqttPort->getValue(), mqttPort->getLength());
  wifiManager.addParameter(&mqttPort_parameter);

  ConfigOption *mqttAuthMode = config.get("mqttAuthMode");
  WiFiManagerParameter mqttAuthMode_parameter(mqttAuthMode->getKey(), mqttAuthMode->getValue(), mqttAuthMode->getLength());
  wifiManager.addParameter(&mqttAuthMode_parameter);

  ConfigOption *mqttTLS = config.get("mqttTLS");
  WiFiManagerParameter mqttTLS_parameter(mqttTLS->getKey(), mqttTLS->getValue(), mqttTLS->getLength());
  wifiManager.addParameter(&mqttTLS_parameter);

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

  ConfigOption *mqttRequestChannel = config.get("mqttRequestChannel");
  WiFiManagerParameter mqttRequestChannel_parameter(mqttRequestChannel->getKey(), mqttRequestChannel->getValue(), mqttRequestChannel->getLength());
  wifiManager.addParameter(&mqttRequestChannel_parameter);

  ConfigOption *mqttCert = config.get("mqttCert");
  WiFiManagerParameter mqttCert_parameter(mqttCert->getKey(), mqttCert->getValue(), mqttCert->getLength());
  wifiManager.addParameter(&mqttCert_parameter);

  ConfigOption *mqttCertKey = config.get("mqttCertKey");
  WiFiManagerParameter mqttCertKey_parameter(mqttCertKey->getKey(), mqttCertKey->getValue(), mqttCertKey->getLength());
  wifiManager.addParameter(&mqttCertKey_parameter);

  ConfigOption *mqttFingerprint = config.get("mqttFingerprint");
  WiFiManagerParameter mqttFingerprint_parameter(mqttFingerprint->getKey(), mqttFingerprint->getValue(), mqttFingerprint->getLength());
  wifiManager.addParameter(&mqttFingerprint_parameter);
  
  wifiManager.setSaveConfigCallback(saveCallback);
  */

  /*
  if(configMode) {
    wifiManager.startConfigPortal(CONFIG_AP_SSID);
  } else {  
    wifiManager.autoConnect(CONFIG_AP_SSID);
  }
  */
  
  /*
  ssid->setValue(ssid_parameter.getValue());
  passkey->setValue(passkey_parameter.getValue());
  encryption->setValue(encryption_parameter.getValue());
  
  mqttDeviceName->setValue(mqttDeviceName_parameter.getValue());
  
  mqttServer->setValue(mqttServer_parameter.getValue());
  mqttPort->setValue(mqttPort_parameter.getValue());
  mqttAuthMode->setValue(mqttAuthMode_parameter.getValue());
  mqttTLS->setValue(mqttTLS_parameter.getValue());
  
  mqttUsername->setValue(mqttUsername_parameter.getValue());
  mqttPassword->setValue(mqttPassword_parameter.getValue());
  
  mqttPublishChannel->setValue(mqttPublishChannel_parameter.getValue());
  mqttSubscribeChannel->setValue(mqttSubscribeChannel_parameter.getValue());
  mqttRequestChannel->setValue(mqttRequestChannel_parameter.getValue());
  
  mqttCert->setValue(mqttCert_parameter.getValue());
  mqttCertKey->setValue(mqttCertKey_parameter.getValue());
  mqttFingerprint->setValue(mqttFingerprint_parameter.getValue());

  if(saveFlag) {
    config.write();
  }
  */

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.get_ssid(), config.get_passkey());
  int WiFiCounter = 0;
  while (WiFi.status() != WL_CONNECTED && WiFiCounter < 30) {
    delay(1000);
    WiFiCounter++;
  }
  
  setNetworkName(config.get_deviceName());
}

WiFiUDP syslogSocket;
void syslogSetup() {
  if(config.get_syslog()) {
    Serial.println("Syslog enabled");
    Syslogger = new Syslog(syslogSocket, config.get_syslogHost(), config.get_syslogPort(), config.get_deviceName(), config.get_deviceName());
    Syslogger->setMinimumSeverity(config.get_syslogLevel());
    Syslogger->send(SYSLOG_INFO, "Device booted.");
  }
}

void pubSubSetup() {  
  pubSub = new PubSub(config.get_mqttServerName(), config.get_mqttPort(), config.get_mqttTLS(), config.get_deviceName());
  pubSub->setCallback(pubSubCallback);
  
  pubSub->setSubscribeChannel(config.get_mqttSubscribeChannel());
  pubSub->setPublishChannel(config.get_mqttPublishChannel());

  pubSub->setAuthMode(config.get_mqttAuthMode());

  Syslogger->send(SYSLOG_INFO, "Loading certificate.");
  pubSub->setFingerprint(config.get_mqttFingerprint());
  
  switch(pubSub->loadCertificate("/client.crt.der")) {
    case E_MQTT_OK:
      Syslogger->send(SYSLOG_INFO, "Certificate loaded.");
      break;
    case E_MQTT_CERT_NOT_LOADED:
      Syslogger->send(SYSLOG_ERROR, "Certificate not loaded.");
      break;
    case E_MQTT_CERT_FILE_NOT_FOUND:
      Syslogger->send(SYSLOG_ERROR, "Couldn't find certificate file.");
      break;
    case E_MQTT_SPIFFS:
      Syslogger->send(SYSLOG_CRITICAL, "Unable to start SPIFFS.");
      break;
  }
  
  Syslogger->send(SYSLOG_INFO, "Loading private key.");    
  switch(pubSub->loadPrivateKey("/client.key.der")) {
     case E_MQTT_OK:
      Syslogger->send(SYSLOG_INFO, "Private key loaded.");
      break;
    case E_MQTT_PRIV_KEY_NOT_LOADED:
      Syslogger->send(SYSLOG_ERROR, "Private key not loaded.");
      break;
    case E_MQTT_PRIV_KEY_FILE_NOT_FOUND:
      Syslogger->send(SYSLOG_ERROR, "Couldn't find private key file.");
      break;
    case E_MQTT_SPIFFS:
      Syslogger->send(SYSLOG_CRITICAL, "Unable to start SPIFFS.");
      break;
  }
}

void setup() {
  //Serial.begin(115200);

  pinMode(RELAY_GND, OUTPUT);
  digitalWrite(RELAY_GND, HIGH);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  
  pinMode(CLOSED_SWITCH, INPUT);
  pinMode(OPENED_SWITCH, INPUT);
  
  
  digitalWrite(RELAY, LOW);
  digitalWrite(RELAY_GND, LOW);
  
  configSetup();
  wifiSetup();
  syslogSetup();
  pubSubSetup();
}

long memdelay = 0;
void loop() {
  if(configMode) {
    return;
  }
  
  pubSub->loop();
  triggerLoop();
  readDoorLoop();

  long memnow = millis();
  if(memnow - memdelay > 1000) {
    Serial.println(ESP.getFreeHeap());
    memdelay = memnow;
  }
}
