#ifndef SYSLOG_h
#define SYSLOG_h

#define SYSLOG_EMERGENCY 0
#define SYSLOG_ALERT 1
#define SYSLOG_CRITICAL 2
#define SYSLOG_ERROR 3
#define SYSLOG_WARNING 4
#define SYSLOG_NOTICE 5
#define SYSLOG_INFO 6
#define SYSLOG_DEBUG 7

#define SYSLOG_KERNEL 0
#define SYSLOG_USER 1
#define SYSLOG_MAIL 2
#define SYSLOG_SYSTEM 3
#define SYSLOG_SECURITY_1 4
#define SYSLOG_SYSLOG 5
#define SYSLOG_LINE_PRINTER 6
#define SYSLOG_NETWORK_NEWS 7
#define SYSLOG_UUCP 8
#define SYSLOG_CLOCK_1 9
#define SYSLOG_SECURITY_2 10
#define SYSLOG_FTP 11
#define SYSLOG_NTP 12
#define SYSLOG_LOG_AUDIT 13
#define SYSLOG_LOG_ALERT 14
#define SYSLOG_CLOCK_2 15
#define SYSLOG_LOCAL_0 16
#define SYSLOG_LOCAL_1 17
#define SYSLOG_LOCAL_2 18
#define SYSLOG_LOCAL_3 19
#define SYSLOG_LOCAL_4 20
#define SYSLOG_LOCAL_5 21
#define SYSLOG_LOCAL_6 22
#define SYSLOG_LOCAL_7 23

#define E_SYSLOG_OK       0x10
#define E_SYSLOG_CONNECT  0x11
#define E_SYSLOG_SEND     0x12
#define E_SYSLOG_DISABLED 0x13

#include <Udp.h>

class Syslog {
  public:
    Syslog();
    Syslog(const char *host, const char *hostname, const char *name);
    Syslog(const char *host, int port, const char *hostname, const char *name);
    Syslog(UDP &udp, const char *host, const char *hostname, const char *name);
    Syslog(UDP &udp, const char *host, int port, const char *hostname, const char *name);

    void setHost(const char *host);
    void setPort(int port);
    void setName(const char *name);
    void setHostname(const char *hostname);
    void setUDP(UDP &udp);

    void setMinimumSeverity(int minimum);
    
    int send(int severity, const char *message);
    int send(int severity, const char *message, int facility);
    
  private:
    const char *host;
    int         port;
    const char *hostname;
    const char *name;
    UDP        *udp;

    int         minimumSeverity;
};

#endif
