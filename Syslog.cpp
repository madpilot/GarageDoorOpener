#include "Syslog.h"
#include <Arduino.h>

Syslog::Syslog() {
  Syslog(NULL, NULL, NULL);
}

Syslog::Syslog(const char *host, const char *hostname, const char *name) {
  this->setHost(host);
  this->setPort(NULL);
  this->setHostname(hostname);
  this->setName(name);
  this->setMinimumSeverity(0);
}

Syslog::Syslog(const char *host, int port, const char *hostname, const char *name) {
  this->setHost(host);
  this->setPort(port);
  this->setHostname(hostname);
  this->setName(name);
  this->setMinimumSeverity(0);
}

Syslog::Syslog(UDP &udp, const char *host, const char *hostname, const char *name) {
  this->setUDP(udp);
  this->setHost(host);
  this->setPort(NULL);
  this->setHostname(hostname);
  this->setName(name);
  this->setMinimumSeverity(0);
}

Syslog::Syslog(UDP &udp, const char *host, int port, const char *hostname, const char *name) {
  this->setHost(host);
  this->setPort(port);
  this->setHostname(hostname);
  this->setName(name);
  this->setUDP(udp);
  this->setMinimumSeverity(0);
}

void Syslog::setHost(const char *host) {
  this->host = host;
}

void Syslog::setPort(int port) {
  this->port = port;
}

void Syslog::setHostname(const char *hostname) {
  this->hostname = hostname;
}

void Syslog::setName(const char *name) {
  this->name = name;
}

void Syslog::setUDP(UDP &udp) {
  this->udp = &udp;
}

void Syslog::setMinimumSeverity(int minimum) {
  this->minimumSeverity = minimum;
}

int Syslog::send(int severity, const char *message) {
  return send(severity, message, SYSLOG_KERNEL);
}

int Syslog::send(int severity, const char *message, int facility) {
  if(this->host == NULL) {
    return E_SYSLOG_DISABLED;
  }

  // Filter out messages above the minimum severity level
  if(severity > this->minimumSeverity) {
    return E_SYSLOG_OK;
  }
  
  int pri = ((facility * 8) + severity);
  String priString = String(pri, DEC);

  String buffer = "<" + priString + ">1 - " + this->hostname + " " + this->name + " - - - " + message;
  int len = buffer.length();
  
  unsigned char payload[len + 1];
  for(int i = 0; i < len; i++)
  {
    payload[i] = (unsigned char)buffer.charAt(i);
  }
  payload[len] = '\0';
  
  if(this->udp->beginPacket(this->host, this->port == NULL ? 514 : this->port)) {
    this->udp->write(payload, len);
    
    if(this->udp->endPacket()) {
      return E_SYSLOG_OK;
    } else {
      return E_SYSLOG_SEND;
    }
  } else {
    return E_SYSLOG_CONNECT;
  }
}

