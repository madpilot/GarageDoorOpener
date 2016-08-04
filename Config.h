#ifndef Config_h
#define Config_h

#define config_result           uint8_t
#define E_CONFIG_OK             0
#define E_CONFIG_FS_ACCESS      1
#define E_CONFIG_FILE_NOT_FOUND 2
#define E_CONFIG_FILE_OPEN      3
#define E_CONFIG_PARSE_ERROR    4
#define E_CONFIG_MAX            5

#define CONFIG_MAX_OPTIONS      17

#include <EEPROM.h>
#include <FS.h>
#include <ArduinoJson.h>

class ConfigOption {
  public:
    ConfigOption(const char *key, const char *value, int maxLength);
    const char *getKey();
    const char *getValue();
    int getLength();
    void setValue(const char *value);
    
  private:
    const char *_key;
    char *_value;
    int _maxLength;
};

class Config {
  public:
    Config();
    config_result addKey(const char *key, int maxLength);
    config_result addKey(const char *key, const char *value, int maxLength);
    config_result read();
    config_result write();
    
    ConfigOption *get(const char *key);
    bool *set(const char *key, const char *value);
  private:
    int _optionCount;
    ConfigOption *_options[CONFIG_MAX_OPTIONS];
    void reset();
};

#endif
