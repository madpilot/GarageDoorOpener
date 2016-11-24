#include "MQTT.h"
#include <mDNSResolver.h>

WiFiClient wifi;
WiFiClientSecure secureWifi;
PubSubClient client;

WiFiUDP udp;
mDNSResolver::Resolver resolver(udp);

PubSub::PubSub(char *server, int port, bool tls, char *deviceName) {  
  if(tls) {
    client.setClient(secureWifi);
  } else {
    client.setClient(wifi);
  }

  resolver.setLocalIP(WiFi.localIP());

  _authMode = AUTH_MODE_NONE;
  _server = server;
  _port = port;
  _deviceName = deviceName;
  
  _qosLevel = 0;
}

void PubSub::setCallback(MQTT_CALLBACK_SIGNATURE) {
  client.setCallback(callback);
}

void PubSub::setAuthMode(int authMode) {
  _authMode = authMode;
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

void PubSub::setFingerprint(const char *fingerprint) {
  _fingerprint = fingerprint;
}

mqtt_result PubSub::loadCertificate(const char *certPath) {
  if(SPIFFS.begin()) {
    File cert = SPIFFS.open(certPath, "r");
    if(cert) {
      if(secureWifi.loadCertificate(cert)) {
        cert.close();
        return E_MQTT_OK;
      } else {
        cert.close();
        return E_MQTT_CERT_NOT_LOADED;
      }
    } else {
      return E_MQTT_CERT_FILE_NOT_FOUND;
    }
  } else {
    return E_MQTT_SPIFFS;
  }
}

mqtt_result PubSub::loadPrivateKey(const char *keyPath) {  
  if(SPIFFS.begin()) {
    File key = SPIFFS.open(keyPath, "r");
    
    if(key) {
      if(secureWifi.loadPrivateKey(key)) {
        key.close();
        return E_MQTT_OK;
      } else {
        key.close();
        return E_MQTT_PRIV_KEY_NOT_LOADED;
      }
    } else {
      return E_MQTT_PRIV_KEY_FILE_NOT_FOUND;
    }
  } else {
    return E_MQTT_SPIFFS;
  }
}

mqtt_result PubSub::connect() {
  if(_authMode == AUTH_MODE_CERTIFICATE) {
    client.disconnect();
    secureWifi.stop();
    
    IPAddress resolved = resolver.search(_server);

    if(resolved == INADDR_NONE) {
      client.setServer(_server, _port);
      if(!secureWifi.connect(_server, _port)) {
        return E_MQTT_CONNECT;
      }
    } else {
      client.setServer(resolved, _port);
      if(!secureWifi.connect(resolved, _port)) {
        return E_MQTT_CONNECT;
      }
    }
    
    if(!secureWifi.verify(_fingerprint, _server)) {
      return E_MQTT_VERIFICATION;
    }
    
    secureWifi.stop();
  }
  
  bool connected = false;
  switch(_authMode) {
    case AUTH_MODE_NONE:
    case AUTH_MODE_CERTIFICATE:
      connected = client.connect(_deviceName);
    case AUTH_MODE_USERNAME:
      connected = client.connect(_deviceName, _username, _password);
      break;
  } 
  
  if(!connected) {
    return E_MQTT_CONNECT;
  }

  if(_subscribeChannel != NULL && _subscribeChannel != "") {
    if(!client.subscribe(_subscribeChannel, _qosLevel)) {
      client.disconnect();
      return E_MQTT_SUBSCRIBE;
    }
  } else {
    client.disconnect();
    return E_MQTT_NO_SUBSCRIBE_CHANNEL;
  }

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
    return E_MQTT_NO_PUBLISH_CHANNEL;
  }
}

long lastConnectionAttempt = 0;
void PubSub::loop() {  
  if(!client.connected()) {
    long now = millis();

    if(now - lastConnectionAttempt > 5000) {
      lastConnectionAttempt = now;
      mqtt_result connectResult = this->connect();
      if(connectResult == E_MQTT_OK) {
        lastConnectionAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
  resolver.loop();
}
