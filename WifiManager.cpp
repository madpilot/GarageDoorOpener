#include "WifiManager.h"

WifiManager::WifiManager(Config* config) {
  this->config = config;
  this->lastConnectionAttempt = 0;
}

wifi_result WifiManager::connect() {  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(config->get_ssid(), config->get_passkey());
  int WiFiCounter = 0;

 return waitForConnection();
}

bool WifiManager::connected() {
  return WiFi.status() == WL_CONNECTED;
}

wifi_result WifiManager::loop() {
  if(!connected()) {
    return connect();
  }
  return E_WIFI_OK;
}

wifi_result WifiManager::waitForConnection() {
  uint8_t status;

  long now = millis();
  lastConnectionAttempt = now;
  while(true) {
    now = millis();
    
    if(now - lastConnectionAttempt > WIFI_TIMEOUT) {
      return WIFI_TIMEOUT;
    }
    
    status = WiFi.status(); 
    
    switch(status) {
      case WL_CONNECTED:
        return E_WIFI_OK;
      case WL_CONNECT_FAILED:
        return E_WIFI_CONNECT_FAILED;    
    }
    
    delay(100);
  }
}

