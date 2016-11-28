#ifndef MQTT_h
#define MQTT_h
#include <Arduino.h>

#include <FS.h>

#include <PubSubClient.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#define mqtt_result                       uint8_t
#define E_MQTT_OK                         0x00
#define E_MQTT_WAITING                    0x01
#define E_MQTT_CONNECT                    0x02
#define E_MQTT_SUBSCRIBE                  0x03
#define E_MQTT_PUBLISH                    0x04
#define E_MQTT_VERIFICATION               0x05
#define E_MQTT_NO_SUBSCRIBE_CHANNEL       0x06
#define E_MQTT_NO_PUBLISH_CHANNEL         0x07
#define E_MQTT_SPIFFS                     0x08

#define E_MQTT_CERT_NOT_LOADED            0x09
#define E_MQTT_CERT_FILE_NOT_FOUND        0x0A
#define E_MQTT_PRIV_KEY_NOT_LOADED        0x0B
#define E_MQTT_PRIV_KEY_FILE_NOT_FOUND    0x0C

#define AUTH_MODE_NONE                    0
#define AUTH_MODE_USERNAME                1
#define AUTH_MODE_CERTIFICATE             2

#define QOS_LEVEL                         0;

class PubSub {
  public:
    PubSub(char *server, int port, bool tls, char *deviceName);

    void setCallback(MQTT_CALLBACK_SIGNATURE);    
    void setSubscribeChannel(const char *channel);
    void setPublishChannel(const char *channel);
    void setAuthMode(int authMode);
    void setAuthentication(const char *username, const char *password);
    void setFingerprint(const char *fingerprint);
    mqtt_result loadCertificate(const char *certPath);
    mqtt_result loadPrivateKey(const char *keyPath);
    
    mqtt_result connect();
    mqtt_result publish(const char *message);
    
    mqtt_result loop();
    
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
    long          lastConnectionAttempt;
};
#endif
