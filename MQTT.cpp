#include "MQTT.h"
#include "Syslogger.h";
#include "mDNSResolver.h";

WiFiClient wifi;
WiFiClientSecure secureWifi;
PubSubClient client;
mDNSResolver::Resolver resolver;

PubSub::PubSub(const char *server, int port, bool tls, const char *deviceName) {  
  if(tls) {
    client.setClient(secureWifi);
  } else {
    client.setClient(wifi);
  }

  _authMode = AUTH_MODE_NONE;
  _server = server;
  _port = port;
  _deviceName = deviceName;
  
  _qosLevel = 0;
  
  client.setServer(_server, _port);
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
        return E_MQTT_OK;
      } else {
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
        return E_MQTT_OK;
      } else {
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
  boolean connected = false;

  resolver.query("mqtt.local");

  
  if(_authMode == AUTH_MODE_CERTIFICATE) {
    if(!secureWifi.connect(_server, _port)) {
      return E_MQTT_CONNECT;
    }
    
    if(!secureWifi.verify(_fingerprint, "mqtt.local")) {
      Syslogger->send(SYSLOG_ERROR, "Fingerprint verification failed.");
      return E_MQTT_VERIFICATION;
    }
    
    secureWifi.stop();
  }

  switch(_authMode) {
    case AUTH_MODE_NONE:
    case AUTH_MODE_CERTIFICATE:
      connected = client.connect(_deviceName);
      break;
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
  mdns_result resolverResult = resolver.loop();
  if(resolverResult != E_MDNS_OK) {
    switch(resolverResult) {
      case E_MDNS_TOO_BIG:
        Syslogger->send(SYSLOG_INFO, "mDNS packet too big.");
        break;
      case E_MDNS_POINTER_OVERFLOW:
        Syslogger->send(SYSLOG_ERROR, "mDNS packet had an overflowing pointer.");
        break;
      case E_MDNS_PACKET_ERROR:
        Syslogger->send(SYSLOG_INFO, "mDNS packet invalid.");
        break;
      case E_MDNS_PARSING_ERROR:
        Syslogger->send(SYSLOG_INFO, "Unable to parse mDNS packet.");
        break;
    }
  }
    
  if(!client.connected()) {
    long now = millis();

    if(now - lastConnectionAttempt > 5000) {
      lastConnectionAttempt = now;
      mqtt_result connectResult = this->connect();
      if(connectResult == E_MQTT_OK) {
        Syslogger->send(SYSLOG_INFO, "Connected to MQTT server.");
        lastConnectionAttempt = 0;
      } else if(connectResult == E_MQTT_WAITING) {
        // Waiting for DNS...
        lastConnectionAttempt = now - 1000;
      } else {
        Syslogger->send(SYSLOG_ERROR, "Connection failed.");
      }
    }
  } else {
    client.loop();
  }
}

