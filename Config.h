#ifndef Config_h
#define Config_h

#define config_result           uint8_t
#define E_CONFIG_OK             0
#define E_CONFIG_FS_ACCESS      1
#define E_CONFIG_FILE_NOT_FOUND 2
#define E_CONFIG_FILE_OPEN      3
#define E_CONFIG_PARSE_ERROR    4

//#include <EEPROM.h>
#include <FS.h>

class Config {
  public:
    Config();

    // Getters
    char* get_ssid();
    char* get_passkey();
    bool get_encryption();
    char* get_deviceName();
    
    char* get_mqttServerName();
    int get_mqttPort();
    
    int get_mqttAuthMode();
    bool get_mqttTLS();

    char* get_mqttUsername();
    char* get_mqttPassword();
    char* get_mqttFingerprint();

    char* get_mqttPublishChannel();
    char* get_mqttSubscribeChannel();

    bool get_syslog();
    char* get_syslogHost();
    int get_syslogPort();
    int get_syslogLevel();

    // Setters
    void set_ssid(char* val);
    void set_passkey(char* val);
    void set_encryption(bool val);
    void set_deviceName(char* val);
    
    void set_mqttServerName(char* val);
    void set_mqttPort(int val);
    
    void set_mqttAuthMode(int val);
    void set_mqttTLS(bool val);

    void set_mqttUsername(char* val);
    void set_mqttPassword(char* val);
    void set_mqttFingerprint(char* val);

    void set_mqttPublishChannel(char* val);
    void set_mqttSubscribeChannel(char* val);

    void set_syslog(bool val);
    void set_syslogHost(char* val);
    void set_syslogPort(int val);
    void set_syslogLevel(int val);

    config_result read();
    config_result write();
    
  private:
    char* ssid;
    char* passkey;
    bool encryption;
    char* deviceName;
    
    char* mqttServerName;
    int mqttPort;
    
    int mqttAuthMode;
    bool mqttTLS;

    char* mqttUsername;
    char* mqttPassword;
    char* mqttFingerprint;

    char* mqttPublishChannel;
    char* mqttSubscribeChannel;

    bool syslog;
    char* syslogHost;
    int syslogPort;
    int syslogLevel;

    bool allocString(char **dest, char *val);
};

#endif
