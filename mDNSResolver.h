#ifndef MDNS_RESOLVER_h
#define MDNS_RESOLVER_h

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define INT_MAX 2147483647
#define MDNS_TARGET_PORT 5353
#define MDNS_SOURCE_PORT 5353
#define MDNS_TTL 255
#define MDNS_RESOLVER_MAX_CACHE 4
#define MDNS_TIMEOUT 5
#define MDNS_MAX_NAME_LEN 255
// Max allowable packet - resolving names should result in small packets.
#define MDNS_MAX_PACKET 256

#define byte unsigned char

namespace mDNSResolver {
  typedef struct Query {
    char *name;
    unsigned int type;
    unsigned int qclass;
    unsigned int unicastResponse;
    bool valid;
  };

  typedef struct Answer {
    char *name;
    unsigned int type;
    unsigned int aclass;
    unsigned int cacheflush;
    unsigned long ttl;
    unsigned int len;
    byte *data;
  };
  
  typedef struct Response {
    char *name;
    IPAddress ipAddress;
    unsigned long int ttl;
    bool waiting;
    unsigned long int timeout;
  };
  
  class Resolver {
    public:
      Resolver();
      ~Resolver();
      void loop();
      Response query(const char *name);
      int search(const char *name);
    private:
      int _cacheCount;
      bool _init;
      Response _cache[MDNS_RESOLVER_MAX_CACHE];
      unsigned long _lastSweep;

      int parseName(char **name, const char *mapped, unsigned int mappedlen);
      int assembleName(byte *buffer, unsigned int len, unsigned int *offset, char **name, unsigned int maxlen);
      int assembleName(byte *buffer, unsigned int len, unsigned int *offset, char **name);

      int skipQuestions(byte *buffer, unsigned int len, unsigned int *offset_ptr);
      int parseAnswer(byte *buffer, unsigned int len, unsigned int *offset_ptr, Answer *a_ptr);
      void parsePacket(byte *buffer, unsigned int len);
      void expire();
      
      Response buildResponse(const char *name);
      Query buildQuery(const char *name);
      void broadcastQuery(Query q);

      void listen();
      
      void insert(Response response);
      void remove(int index);
  };
};
#endif
