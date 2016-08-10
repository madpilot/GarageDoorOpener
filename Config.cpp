#include "Config.h"

#define CONFIG_FILE_PATH "/config.dat"

ConfigOption::ConfigOption(const char *key, const char *value, int maxLength) {
  _key = key;
  _maxLength = maxLength;
  setValue(value);
}

const char *ConfigOption::getKey() {
  return _key;
}

const char *ConfigOption::getValue() {
  return _value;
}

int ConfigOption::getLength() {
  return _maxLength;
}

void ConfigOption::setValue(const char *value) {  
  _value = (char *)malloc(sizeof(char) * (_maxLength + 1));

  // NULL out the string, including a terminator
  for(int i = 0; i < _maxLength + 1; i++) {
    _value[i] = '\0';
  }

  if(value != NULL) {
    strncpy(_value, value, _maxLength);
  }
}

Config::Config() {
  _optionCount = 0;
};

config_result Config::addKey(const char *key, int maxLength) {
  return addKey(key, NULL, maxLength);
}

config_result Config::addKey(const char *key, const char *value, int maxLength) {
  if(_optionCount == CONFIG_MAX_OPTIONS) {
    return E_CONFIG_MAX;
  }
  _options[_optionCount] = new ConfigOption(key, value, maxLength);
  _optionCount += 1;
  
  return E_CONFIG_OK;
}

config_result Config::read() {
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
}

config_result Config::write() {
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
  
  return E_CONFIG_FS_ACCESS;
}

/*
 * Returns the config option that maps to the supplied key.
 * Returns NULL if not found
 */
ConfigOption *Config::get(const char *key) {
  for(int i = 0; i < _optionCount; i++) {
    if(strcmp(_options[i]->getKey(), key) == 0) {
      return _options[i];  
    }
  }
  
  return NULL;
}

