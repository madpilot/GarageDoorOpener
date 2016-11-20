#include "Config.h"

#define CONFIG_FILE_PATH "/config.dat"

Config::Config() {
  this->ssid = NULL;
  this->passkey = NULL;
  this->encryption = false;
  this->deviceName = NULL;
  
  this->mqttServerName = NULL;
  this->mqttPort = 1883;
  
  this->mqttAuthMode = 0;
  this->mqttTLS = false;

  this->mqttUsername = NULL;
  this->mqttPassword = NULL;
  this->mqttFingerprint = NULL;

  this->mqttPublishChannel = NULL;
  this->mqttSubscribeChannel = NULL;

  this->syslog = false;
  this->syslogHost = NULL;
  this->syslogPort = 514;
  this->syslogLevel = 6;

  this->set_ssid("Burntos");
  this->set_passkey("BellsDiner");
  this->set_encryption(true);
  this->set_deviceName("garage");
  this->set_mqttServerName("mqtt.local");
  this->set_mqttPort(8883);
  this->set_mqttAuthMode(2);
  this->set_mqttTLS(true);
  this->set_mqttFingerprint("CF 7A A1 E3 0E 50 D9 C2 67 60 6E 44 2A 16 5E B1 AF D5 CB 91");
  this->set_mqttPublishChannel("home-assistant/cover");
  this->set_mqttSubscribeChannel("home-assistant/cover/set");
  this->set_syslog(true);
  this->set_syslogHost("192.168.1.2");
  this->set_syslogLevel(7);
}

char* Config::get_ssid() {
  if(ssid == NULL) {
    return "";
  } else {
    return ssid;
  }
}

char* Config::get_passkey() {
  return passkey;
}

bool Config::get_encryption() {
  return encryption;
}

char* Config::get_deviceName() {
  return deviceName;
}

char* Config::get_mqttServerName() {
  return mqttServerName;
}

int Config::get_mqttPort() {
  return mqttPort;
}

int Config::get_mqttAuthMode() {
  return mqttAuthMode;
}

bool Config::get_mqttTLS() {
  return mqttTLS;
}

char* Config::get_mqttUsername() {
  return mqttUsername;
}

char* Config::get_mqttPassword() {
  return mqttPassword;
}

char* Config::get_mqttFingerprint() {
  return mqttFingerprint;
}

char* Config::get_mqttPublishChannel() {
  return mqttPublishChannel;
}

char* Config::get_mqttSubscribeChannel() {
  return mqttSubscribeChannel;
}

bool Config::get_syslog() {
  return syslog;
}

char* Config::get_syslogHost() {
  return syslogHost;
}

int Config::get_syslogPort() {
  return syslogPort;
}

int Config::get_syslogLevel() {
  return syslogLevel;
}

// Setters
void Config::set_ssid(char* val) {
  allocString(&this->ssid, val);
}

void Config::set_passkey(char* val) {
  allocString(&this->passkey, val);
}

void Config::set_encryption(bool val) {
  this->encryption = val;
}

void Config::set_deviceName(char* val) {
  allocString(&this->deviceName, val);
}

void Config::set_mqttServerName(char* val) {
  allocString(&this->mqttServerName, val);
}

void Config::set_mqttPort(int val) {
  this->mqttPort = val;
}

void Config::set_mqttAuthMode(int val) {
  this->mqttAuthMode = val;
}

void Config::set_mqttTLS(bool val) {
  this->mqttTLS = val;
}

void Config::set_mqttUsername(char* val) {
  allocString(&this->mqttUsername, val);
}

void Config::set_mqttPassword(char* val) {
  allocString(&this->mqttPassword, val);
}

void Config::set_mqttFingerprint(char* val) {
  allocString(&this->mqttFingerprint, val);
}

void Config::set_mqttPublishChannel(char* val) {
  allocString(&this->mqttPublishChannel, val);
}

void Config::set_mqttSubscribeChannel(char* val) {
  allocString(&this->mqttSubscribeChannel, val);
}

void Config::set_syslog(bool val) {
  this->syslog = val;
}

void Config::set_syslogHost(char* val) {
  allocString(&this->syslogHost, val);
}

void Config::set_syslogPort(int val) {
  this->syslogPort = val;
}

void Config::set_syslogLevel(int val) {
  this->syslogLevel = val;
}

//426206
bool Config::allocString(char **dest, char *val) { 
  if((*dest) != NULL) {
    free((*dest));
  }
  
  (*dest) = (char*)malloc(sizeof(char *) * (strlen(val) + 1));
  if((*dest) == NULL) {
    return false;
  }
  strcpy((*dest), val);
  return true;
}

config_result Config::read() {
  /*
  if (SPIFFS.begin()) {
    if (SPIFFS.exists(CONFIG_FILE_PATH)) {
      File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
      
      if (configFile) {        
        int i = 0;
        int offset = 0;
        int length = 0;

        for(i = 0; i < _optionCount; i++) {
          length += _options[i]->getLength();
        }

        if(length != configFile.size()) {
          return E_CONFIG_PARSE_ERROR;
        }

        uint8_t *content = (uint8_t *)malloc(sizeof(uint8_t) * length);
        configFile.read(content, length);
        
        for(i = 0; i < _optionCount; i++) {
          // Because we know the right number of bytes gets copied,
          // and it gets null terminated,
          // we can just pass in an offset pointer to save a temporary variable
          _options[i]->setValue((const char *)(content + offset));
          offset += _options[i]->getLength();
        }
        
        configFile.close();             
        free(content);
        
        return E_CONFIG_OK;
      } else {
        configFile.close();
        return E_CONFIG_FILE_OPEN;
      }
    } else {
      return E_CONFIG_FILE_NOT_FOUND;
    }
  } else {
    return E_CONFIG_FS_ACCESS;
  }
  */
}

config_result Config::write() {
  /*
  if (SPIFFS.begin()) {
    int i = 0;
    int offset = 0;
    int length = 0;

    for(i = 0; i < _optionCount; i++) {
      length += _options[i]->getLength();
    }

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w+");
    if(configFile) {
      uint8_t *content = (uint8_t *)malloc(sizeof(uint8_t) * length);
      for(i = 0; i < _optionCount; i++) {
        memcpy(content + offset, _options[i]->getValue(), _options[i]->getLength());
        offset += _options[i]->getLength();
      }
      
      configFile.write(content, length);
      configFile.close();
      
      free(content);
      return E_CONFIG_OK;
    } else {
      return E_CONFIG_FILE_OPEN;
    }
  }
  */
  return E_CONFIG_FS_ACCESS;
}
