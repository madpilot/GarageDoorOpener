#include "Config.h"

#define CONFIG_FILE_PATH "/config.dat"

Config::Config() {
  this->ssid = NULL;
  this->passkey = NULL;
  this->encryption = 7;
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

  this->dhcp = true;
  this->staticIP = NULL;
  this->staticDNS = NULL;
  this->staticGateway = NULL;
  this->staticSubnet = NULL;

	this->set_ssid("");
	this->set_passkey("");
	this->set_deviceName("");
	this->set_mqttServerName("");
	this->set_mqttUsername("");
	this->set_mqttPassword("");
	this->set_mqttFingerprint("");
	this->set_mqttPublishChannel("");
	this->set_mqttSubscribeChannel("");
	this->set_syslogHost("");
  this->set_staticIP("");
  this->set_staticDNS("");
  this->set_staticGateway("");
  this->set_staticSubnet("");
}

char* Config::get_ssid() {
  return ssid;
}

char* Config::get_passkey() {
  return passkey;
}

int Config::get_encryption() {
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

bool Config::get_dhcp() {
  return dhcp;
}

char* Config::get_staticIP() {
  return staticIP;
}

char* Config::get_staticDNS() {
  return staticDNS;
}

char* Config::get_staticGateway() {
  return staticGateway;
}

char* Config::get_staticSubnet() {
  return staticSubnet;
}

// Setters
void Config::set_ssid(const char* val) {
  allocString(&this->ssid, val);
}

void Config::set_passkey(const char* val) {
  allocString(&this->passkey, val);
}

void Config::set_encryption(int val) {
  this->encryption = val;
}

void Config::set_deviceName(const char* val) {
  allocString(&this->deviceName, val);
}

void Config::set_mqttServerName(const char* val) {
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

void Config::set_mqttUsername(const char* val) {
  allocString(&this->mqttUsername, val);
}

void Config::set_mqttPassword(const char* val) {
  allocString(&this->mqttPassword, val);
}

void Config::set_mqttFingerprint(const char* val) {
  allocString(&this->mqttFingerprint, val);
}

void Config::set_mqttPublishChannel(const char* val) {
  allocString(&this->mqttPublishChannel, val);
}

void Config::set_mqttSubscribeChannel(const char* val) {
  allocString(&this->mqttSubscribeChannel, val);
}

void Config::set_syslog(bool val) {
  this->syslog = val;
}

void Config::set_syslogHost(const char* val) {
  allocString(&this->syslogHost, val);
}

void Config::set_syslogPort(int val) {
  this->syslogPort = val;
}

void Config::set_syslogLevel(int val) {
  this->syslogLevel = val;
}

void Config::set_dhcp(bool val) {
  this->dhcp = val;
}

void Config::set_staticIP(const char* val) {
  allocString(&this->staticIP, val);
}

void Config::set_staticDNS(const char* val) {
  allocString(&this->staticDNS, val);
}

void Config::set_staticGateway(const char* val) {
  allocString(&this->staticGateway, val);
}

void Config::set_staticSubnet(const char* val) {
  allocString(&this->staticSubnet, val);
}

bool Config::allocString(char **dest, const char *val) {
  if((*dest) != NULL) {
    free((*dest));
  }

  int len = strlen(val);
  // Strings can't be longer than 255 characters. If they are, truncate them
  if(len > 255) {
    len = 255;
  }

  (*dest) = (char*)malloc(sizeof(char) * (len + 1));
  if((*dest) == NULL) {
    return false;
  }
  strncpy((*dest), val, len);
  return true;
}

int Config::estimateSerializeBufferLength() {
  int size = 7;
  size += strlen(ssid) + 1;
  size += strlen(passkey) + 1;
  size += strlen(deviceName) + 1;
  size += strlen(mqttServerName) + 1;
  size += strlen(mqttUsername) + 1;
  size += strlen(mqttPassword) + 1;
  size += strlen(mqttFingerprint) + 1;
  size += strlen(mqttPublishChannel) + 1;
  size += strlen(mqttSubscribeChannel) + 1;
  size += strlen(syslogHost) + 1;
  size += strlen(staticIP) + 1;
  size += strlen(staticDNS) + 1;
  size += strlen(staticGateway) + 1;
  size += strlen(staticSubnet) + 1;
  return size;
}

void Config::serializeString(unsigned char *buffer, char *string, int *offset) {
  if(string == NULL) {
    buffer[(*offset)++] = 0;
    return;
  }

  int len = strlen(string);
  buffer[(*offset)++] = len;
  memcpy(buffer + (*offset), string, len);
  (*offset) += len;
}

config_result Config::deserializeString(unsigned char *buffer, int bufferlen, char **string, int *offset) {
  int len = buffer[(*offset)++];

  if((*offset) + len > bufferlen) {
    return E_CONFIG_PARSE_ERROR;
  }

  if(*string != NULL) {
    free(*string);
  }

  *string = (char *)malloc(sizeof(char) * (len + 1));

  if(*string == NULL) {
    return E_CONFIG_OUT_OF_MEMORY;
  }

  memcpy(*string, buffer + (*offset), len);
  (*string)[len] = 0;

  (*offset) += len;

  return E_CONFIG_OK;
}

int Config::serialize(unsigned char *buffer) {
  buffer[0] = 0; // Config version number

  // Reserve a byte for booleans and flags
  // bit 0: Encryption
  // bit 1: Encryption
  // bit 2: Encryption
  // bit 3: dhcp
  // bit 4: syslog
  // bit 5: mqttAuthMode LSB
  // bit 6: mqttAuthMode MSB
  // bit 7: mqttTLS
  buffer[1] = 0;
  buffer[1] = buffer[1] | (encryption & 0x07);
  buffer[1] = buffer[1] | (dhcp & 0x01) << 3;
  buffer[1] = buffer[1] | (syslog & 0x01) << 4;
  buffer[1] = buffer[1] | (mqttAuthMode & 0x03) << 5;
  buffer[1] = buffer[1] | (mqttTLS & 0x01) << 7;

  // mqttPort - 16 bit number
  buffer[2] = (mqttPort >> 8) & 0xFF;
  buffer[3] = mqttPort & 0xFF;

  // syslogPort - 16 bit number
  buffer[4] = (syslogPort >> 8) & 0xff;
  buffer[5] = syslogPort & 0xFF;

  // syslogLevel - 8 bit number
  buffer[6] = syslogLevel & 0xFF;

  int offset = 7;
  serializeString(buffer, ssid, &offset);
  serializeString(buffer, passkey, &offset);
  serializeString(buffer, deviceName, &offset);
  serializeString(buffer, mqttServerName, &offset);
  serializeString(buffer, mqttUsername, &offset);
  serializeString(buffer, mqttPassword, &offset);
  serializeString(buffer, mqttFingerprint, &offset);
  serializeString(buffer, mqttPublishChannel, &offset);
  serializeString(buffer, mqttSubscribeChannel, &offset);
  serializeString(buffer, syslogHost, &offset);
  serializeString(buffer, staticIP, &offset);
  serializeString(buffer, staticDNS, &offset);
  serializeString(buffer, staticGateway, &offset);
  serializeString(buffer, staticSubnet, &offset);

  return offset;
}

config_result Config::deserialize(unsigned char *buffer, int length) {
  if(buffer[0] != 0) {
    return E_CONFIG_UNKNOWN_VERSION;
  }

  if(length < 17) {
    return E_CONFIG_CORRUPT;
  }

  encryption = buffer[1] & 0x07;
  dhcp = (buffer[1] >> 3) & 0x01;
  syslog = (buffer[1] >> 4) & 0x01;
  mqttAuthMode = (buffer[1] >> 5) & 0x03;
  mqttTLS = (buffer[1] >> 7) & 0x01;

  mqttPort = (buffer[2] << 8) + buffer[3];
  syslogPort = (buffer[4] << 8) + buffer[5];

  syslogLevel = buffer[6];

  int offset = 7;

  config_result result;

  result = deserializeString(buffer, length, &ssid, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &passkey, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &deviceName, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &mqttServerName, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &mqttUsername, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &mqttPassword, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &mqttFingerprint, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &mqttPublishChannel, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &mqttSubscribeChannel, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &syslogHost, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticIP, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticDNS, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticGateway, &offset);
  if(result != E_CONFIG_OK) return result;
  result = deserializeString(buffer, length, &staticSubnet, &offset);
  if(result != E_CONFIG_OK) return result;

  return E_CONFIG_OK;
}



config_result Config::read() {
  if (SPIFFS.begin()) {
    if (SPIFFS.exists(CONFIG_FILE_PATH)) {
      File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");

      if (configFile) {
        int length = configFile.size();

        unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * length);
        if(buffer == NULL) {
          return E_CONFIG_OUT_OF_MEMORY;
        }

        configFile.read(buffer, length);
        deserialize(buffer, length);

        free(buffer);

        configFile.close();
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
}

config_result Config::write() {
  int bufferLength = estimateSerializeBufferLength();
  unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * bufferLength);
  if(buffer == NULL) {
    return E_CONFIG_OUT_OF_MEMORY;
  }

  int length = serialize(buffer);

  if (SPIFFS.begin()) {
    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w+");

    if(configFile) {
      configFile.write(buffer, length);
      configFile.close();

      free(buffer);
      return E_CONFIG_OK;
    } else {
      free(buffer);
      return E_CONFIG_FILE_OPEN;
    }
  }
  free(buffer);
  return E_CONFIG_FS_ACCESS;
}
