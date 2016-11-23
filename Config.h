#ifndef Config_h
#define Config_h

#define config_result             uint8_t
#define E_CONFIG_OK               0
#define E_CONFIG_FS_ACCESS        1
#define E_CONFIG_FILE_NOT_FOUND   2
#define E_CONFIG_FILE_OPEN        3
#define E_CONFIG_PARSE_ERROR      4
#define E_CONFIG_OUT_OF_MEMORY    5
#define E_CONFIG_UNKNOWN_VERSION  6
#define E_CONFIG_CORRUPT          7

#include <FS.h>

class Config {
  public:
    Config();

    // Getters
    char* get_ssid();
    char* get_passkey();
    int get_encryption();
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

    bool get_dhcp();
    char* get_staticIP();
    char* get_staticDNS();
    char* get_staticGateway();
    char* get_staticSubnet();

    // Setters
    void set_ssid(const char* val);
    void set_passkey(const char* val);
    void set_encryption(int val);
    void set_deviceName(const char* val);

    void set_mqttServerName(const char* val);
    void set_mqttPort(int val);

    void set_mqttAuthMode(int val);
    void set_mqttTLS(bool val);

    void set_mqttUsername(const char* val);
    void set_mqttPassword(const char* val);
    void set_mqttFingerprint(const char* val);

    void set_mqttPublishChannel(const char* val);
    void set_mqttSubscribeChannel(const char* val);

    void set_syslog(bool val);
    void set_syslogHost(const char* val);
    void set_syslogPort(int val);
    void set_syslogLevel(int val);


    void set_dhcp(bool val);;
    void set_staticIP(const char* val);
    void set_staticDNS(const char* val);
    void set_staticGateway(const char* val);
    void set_staticSubnet(const char* val);

    config_result write();
    config_result read();

  private:
    char* ssid;
    char* passkey;
    int encryption;
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

    bool dhcp;
    char* staticIP;
    char* staticDNS;
    char* staticGateway;
    char* staticSubnet;

    bool allocString(char **dest, const char *val);
    config_result deserialize(unsigned char *buffer, int length);
    config_result deserializeString(unsigned char *buffer, int bufferlen, char **string, int *offset);

    int serialize(unsigned char *buffer);
    void serializeString(unsigned char *buffer, char *string, int *offset);

    int estimateSerializeBufferLength();
};



#endif
