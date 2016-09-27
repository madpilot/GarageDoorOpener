#include "MQTT.h"
#include "Syslogger.h";

WiFiClient wifi;
WiFiClientSecure secureWifi;
PubSubClient client;

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
  
  client.setServer(server, port);
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
  //Syslogger->send(SYSLOG_INFO, "Connecting to MQTT server.");

  boolean connected = false;

  if(_authMode == AUTH_MODE_CERTIFICATE) {
    if(!secureWifi.connect(_server, _port)) {
      return E_MQTT_CONNECT;
    }
    
  
    if(secureWifi.verify(_fingerprint, "myles-xps13.local")) {
      //Syslogger->send(SYSLOG_INFO, "MQTT server verified.");
    } else {
      //Syslogger->send(SYSLOG_ALERT, "MQTT server failed fingerprint check!"); 
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
    //Syslogger->send(SYSLOG_ERROR, "Unable to connect to MQTT server. Will try again in 5 seconds.");
    return E_MQTT_CONNECT;
  }

  if(_subscribeChannel != NULL && _subscribeChannel != "") {
    if(!client.subscribe(_subscribeChannel, _qosLevel)) {
      //Syslogger->send(SYSLOG_ERROR, "Unable to subscribed to channel.");
      //Serial.println(_subscribeChannel);
      client.disconnect();
      return E_MQTT_SUBSCRIBE;
    } else {
      //Syslogger->send(SYSLOG_INFO, "Subscribed to channel.");
    }
  } else {
    //Syslogger->send(SYSLOG_ERROR, "No subscribe channel set.");
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
    //Syslogger->send(SYSLOG_ERROR, "No publish channel set.");
  }
}

long lastConnectionAttempt = 0;
void PubSub::loop() {
  if(!client.connected()) {
    long now = millis();

    if(now - lastConnectionAttempt > 5000) {
      Syslogger->send(SYSLOG_INFO, "Attempting connection to MQTT server.");

      lastConnectionAttempt = now;
      if(this->connect() == E_MQTT_OK) {
        Syslogger->send(SYSLOG_INFO, "Connected to MQTT server.");
        lastConnectionAttempt = 0;
      } else {
        Syslogger->send(SYSLOG_ERROR, "Connection failed.");
      }
    }
  } else {
    client.loop();
  }
}

