#ifndef WIFI_MANAGER_h
#define WIFI_MANAGER_h

#include <ESP8266WiFi.h>
#include "Config.h"

#define WIFI_TIMEOUT              30000

#define wifi_result               uint8_t
#define E_WIFI_OK                 0x00
#define E_WIFI_CONNECT_FAILED     0x01
#define E_WIFI_TIMEOUT            0x02

class WifiManager {
  public:
    WifiManager(Config* config);
    wifi_result connect();
    bool connected();
    wifi_result loop();
  private:
    wifi_result waitForConnection();
    long lastConnectionAttempt;
    Config* config;
};
#endif
