#ifndef CONFIG_MANAGER_h
#define CONFIG_MANAGER_h

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>


#include "Config.h"
#define E_CONFIG_MODE 8

class ConfigManager {
  public:
    ConfigManager(Config* config);
    config_result setup();
    bool configMode();
    void setConfigMode();
    void loop();
  private:
    bool _configMode;
    config_result loadConfig();
    Config* config;
    ESP8266WebServer *httpServer;
    ESP8266HTTPUpdateServer *httpUpdater;
};
#endif
