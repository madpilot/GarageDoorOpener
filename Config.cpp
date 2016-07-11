#include "Config.h"

#define CONFIG_FILE_PATH "/config.json"

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
  _value = new char[_maxLength + 1];
  
  for(int i = 0; i < _maxLength; i++) {
    _value[i] = '\0';
  }

  if(value != NULL) {
    strncpy(_value, value, _maxLength);
  }
}

Config::Config() {
  
};

config_result Config::addKey(const char *key, int maxLength) {
  return addKey(key, NULL, maxLength);
}

config_result Config::addKey(const char *key, const char *value, int maxLength) {
  if(_optionCount == CONFIG_MAX_OPTIONS - 1) {
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
        size_t size = configFile.size();
        std::unique_ptr<char[]> json(new char[size]);
        configFile.readBytes(json.get(), size);

        DynamicJsonBuffer buf;
        JsonObject &root = buf.parseObject(json.get());
        
        if(root.success()) {
          for(int i = 0; i < _optionCount; i++) {
            _options[i]->setValue(root[_options[i]->getKey()]);
          }
          
          configFile.close();             
          return E_CONFIG_OK;
        } else {
          configFile.close();
          return E_CONFIG_PARSE_ERROR;
        }        
      } else {
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
  DynamicJsonBuffer buf;
  JsonObject &root = buf.createObject();

  for(int i = 0; i < _optionCount; i++) {
    root[_options[i]->getKey()] = _options[i]->getValue();
  }
  
  if (SPIFFS.begin()) {
    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w+");
    root.printTo(configFile);
    configFile.close();
    return E_CONFIG_OK;
  } else {
    return E_CONFIG_FS_ACCESS;
  }
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

