 #include "ConfigManager.h"

 #define CONFIG_MANAGER_PATH "/firmware"
 #define CONFIG_MANAGER_USERNAME "admin"
 #define CONFIG_MANAGER_PASSWORD "admin"

ConfigManager::ConfigManager(Config *config) {
  this->_configMode = false;
  this->config = config;
}

config_result ConfigManager::setup() {
  config_result result = loadConfig();
  
  if(result != E_CONFIG_OK) {
    //  Config not read correctly. Bail out. We don't want a corrupt config file opening up an attack vector
    this->setConfigMode();
    return result;
  }

  if(this->configMode()) {
    this->httpServer = new ESP8266WebServer(80);
    this->httpUpdater = new ESP8266HTTPUpdateServer();

    Serial.begin(115200);
    Serial.println();
    Serial.println("Entering Config Mode...");
    WiFi.mode(WIFI_AP_STA);

    do {
      Serial.println("Connecting to WiFI");
      WiFi.begin(config->get_ssid(), config->get_passkey());
      
    } while (WiFi.waitForConnectResult() != WL_CONNECTED);
  
    MDNS.begin(config->get_deviceName());
    httpUpdater->setup(httpServer, CONFIG_MANAGER_PATH, CONFIG_MANAGER_USERNAME, CONFIG_MANAGER_PASSWORD);
    httpServer->begin();

    MDNS.addService("http", "tcp", 80);

    Serial.printf("Open http://%s.local%s in your browser\n", config->get_deviceName(), CONFIG_MANAGER_PATH);
    
    return E_CONFIG_MODE;
  }
  
  return result;
}

bool ConfigManager::configMode() {
  return this->_configMode;
}

void ConfigManager::setConfigMode() {
  this->_configMode = true;
}

config_result ConfigManager::loadConfig() {
  config_result result = config->read();
  
  switch(result) {
    case E_CONFIG_OK:
      Serial.println("Config read");
      break;
    case E_CONFIG_FS_ACCESS:
      Serial.println("E_CONFIG_FS_ACCESS: Couldn't access file system");
      break;
    case E_CONFIG_FILE_NOT_FOUND:
      Serial.println("E_CONFIG_FILE_NOT_FOUND: File not found");
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println("E_CONFIG_FILE_OPEN: Couldn't open file");
      break;
    case E_CONFIG_PARSE_ERROR:
      Serial.println("E_CONFIG_PARSE_ERROR: File was not parsable");
      break;
    }
    
    return result;
}

void ConfigManager::loop() {
  this->httpServer->handleClient();
}

