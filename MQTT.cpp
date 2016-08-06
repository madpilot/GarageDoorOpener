#include "MQTT.h"

WiFiClient wifi;
WiFiClientSecure secureWifi;
PubSubClient client;

PubSub::PubSub(const char *server, int port, bool tls, const char *deviceName) {  
  if(tls) {
    client.setClient(secureWifi);
  } else {
    client.setClient(wifi);
  }
  
  //wifi->setCertificate(client_cert, client_cert_len);
  //wifi->setPrivateKey(client_key, client_key_len);
  
  _server = server;
  _port = port;
  _deviceName = deviceName;
  _qosLevel = 0;
  
  client.setServer(server, port);
}

void PubSub::setCallback(MQTT_CALLBACK_SIGNATURE) {
  client.setCallback(callback);
}

void PubSub::setSubscribeChannel(const char *channel) {
  _subscribeChannel = channel;
}

void PubSub::setPublishChannel(const char *channel) {
  _publishChannel = channel;
}

void PubSub::setAuthentication(const char *username, const char *password) {
  _username = username;
  _password = password;
}

void PubSub::setCertificate(const char *cert, const char *certKey) {
  _cert = cert;
  _certKey = certKey;
}

mqtt_result PubSub::connect() {
  Serial.print("Connecting to MQTT server...");

  if(!client.connect(_deviceName)) {
    Serial.println("\nCouldn't connect to MQTT server. Will try again in 5 seconds.");
    return E_MQTT_CONNECT;
  }

  if(_subscribeChannel != NULL && _subscribeChannel != "") {
    if(!client.subscribe(_subscribeChannel, _qosLevel)) {
      Serial.print("\nUnable to subscribe to ");
      Serial.println(_subscribeChannel);
      client.disconnect();
      return E_MQTT_SUBSCRIBE;
    }
  } else {
    Serial.println("No subscribe channel set");
  }

  Serial.println(" Connected.");
  return E_MQTT_OK;
}

mqtt_result PubSub::publish(const char *message) {
  if(_publishChannel != NULL && _publishChannel != "") {
    if(client.publish(_publishChannel, message)) {
      return E_MQTT_OK;
    } else {
      return E_MQTT_PUBLISH;
    }
  } else {
    Serial.println("No publish channel set");
  }
}

int lastConnectionAttempt = 0;

void PubSub::loop() { 
  if(!client.connected()) {
    Serial.println("Not connected");

    long now = millis();

    if(now - lastConnectionAttempt > 5000) {
      lastConnectionAttempt = now;
      if(this->connect() == E_MQTT_OK) {
        Serial.println("Connected");
        lastConnectionAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
}

PubSub::~PubSub() {
  //free(client);
}

