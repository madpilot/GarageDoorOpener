#ifndef MQTT_h
#define MQTT_h
#include "Arduino.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#define mqtt_result             uint8_t
#define E_MQTT_OK               0
#define E_MQTT_CONNECT          1
#define E_MQTT_SUBSCRIBE        2
#define E_MQTT_PUBLISH          3

#define QOS_LEVEL 0;

class PubSub {
  public:
    PubSub(const char *server, int port, bool tls, const char *deviceName);

    void setCallback(MQTT_CALLBACK_SIGNATURE);    
    void setSubscribeChannel(const char *channel);
    void setPublishChannel(const char *channel);
    void setAuthentication(const char *username, const char *password);
    void setCertificate(const char *cert, const char *certKey);
    
    mqtt_result connect();
    mqtt_result publish(const char *message);
    
    void loop();
    
    ~PubSub();
    
  private:
    const char   *_server;
    int           _port;
    bool           _tls;
    const char   *_deviceName;
    const char   *_username;
    const char   *_password;
    const char   *_cert;
    const char   *_certKey;
    const char   *_subscribeChannel;
    const char   *_publishChannel;
    int           _qosLevel;
    
    PubSubClient client;
    long          lastConnectionAttempt;
};
#endif
