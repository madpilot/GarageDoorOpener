#ifndef MQTT_h
#define MQTT_h
#include <Arduino.h>

#include <FS.h>

#include <PubSubClient.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#define mqtt_result             uint8_t
#define E_MQTT_OK               0
#define E_MQTT_CONNECT          1
#define E_MQTT_SUBSCRIBE        2
#define E_MQTT_PUBLISH          3
#define E_MQTT_VERIFICATION     4

#define AUTH_MODE_NONE 0
#define AUTH_MODE_USERNAME 1
#define AUTH_MODE_CERTIFICATE 2

#define QOS_LEVEL 0;

class PubSub {
  public:
    PubSub(const char *server, int port, bool tls, const char *deviceName);

    void setCallback(MQTT_CALLBACK_SIGNATURE);    
    void setSubscribeChannel(const char *channel);
    void setPublishChannel(const char *channel);
    void setAuthMode(int authMode);
    void setAuthentication(const char *username, const char *password);
    void setFingerprint(const char *fingerprint);
    void loadCertificate(const char *certPath);
    void loadPrivateKey(const char *keyPath);
    
    mqtt_result connect();
    mqtt_result publish(const char *message);
    
    void loop();
    
    ~PubSub();
    
  private:
    const char   *_server;
    int           _port;
    bool          _tls;
    int           _authMode;
    const char   *_deviceName;
    const char   *_username;
    const char   *_password;
    const char   *_fingerprint;
    const char   *_subscribeChannel;
    const char   *_publishChannel;
    int           _qosLevel;
    
    //PubSubClient  client;
    long          lastConnectionAttempt;
};
#endif
