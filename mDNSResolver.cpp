#include "mDNSResolver.h"

namespace mDNSResolver {  
  WiFiUDP Udp;

  Resolver::Resolver() {
    _cacheCount = 0;
    _lastSweep = 0;
    _init = false;
  }

  Resolver::~Resolver() {
    
  }

  void Resolver::loop() {
    listen();
    //expire();
  }

  // Work out how many bytes are dedicated to questions. Since we aren't answering questions, they can be skipped
  int Resolver::skipQuestions(byte *buffer, unsigned int len, unsigned int *offset) {
    unsigned int questionCount = (buffer[4] << 8) + buffer[5];
    Serial.print("Question Count: ");
    Serial.println(questionCount);
    
    (*offset) = 12;
    // Name payload looks like this:
    // 4xxxx5xxxxx0qtqc
    for(int i = 0; i < questionCount; i++) {
      unsigned int questionSize = 0;

      while(buffer[*offset] != '\0') {
        // If it's a pointer, add two to the counter
        if(buffer[*offset] == 0xc0) {
          questionSize += 2;
          break;
        } else {
          questionSize += (unsigned int)buffer[*offset] + 1; 
        }

        if(questionSize > 255) {
          return -1;
        }
      }
      
      (*offset) += questionSize + 4; // 2 bytes for the qtypes and 2 bytes qclass
    }
    
    return 0;
  }

  // Len should not include the NULL byte string terminator
  int Resolver::parseName(char **name, const char *mapped, unsigned int len) {
    // This is not an off-by-one error - the unparsed name is one character longer than the parsed one.
    // So we don't need to add a byte for the NULL terminator.
    *name = (char *)malloc(sizeof(char) * (len - 1));

    unsigned int namePointer = 0;
    unsigned int mapPointer = 0;
    
    while(mapPointer < len) {
      int labelLength = mapped[mapPointer++];
      
      if(namePointer != 0) {
        (*name)[namePointer++] = '.';
      }

      for(int i = 0; i < labelLength; i++) {
        (*name)[namePointer++] = mapped[mapPointer++];
      }
    }
    
    (*name)[len - 1] = '\0';
    return 0;
  }

  int Resolver::assembleName(byte *buffer, unsigned int len, unsigned int *offset, char **name, unsigned int maxlen) {
    char *scratch = (char *)malloc(sizeof(char) * (MDNS_MAX_NAME_LEN + 1));
    unsigned int index = 0;

    while(buffer[*offset] != '\0' && index < maxlen) {
      if(buffer[(*offset)] == 0xc0) {
        char *pointer;
        unsigned int pointerOffset = ((buffer[(*offset)++] & 0x3f) << 8) + buffer[*offset];
        assembleName(buffer, len, &pointerOffset, &pointer);
        
        unsigned int pointerLen = strlen(pointer);
        memcpy(scratch + index, pointer, pointerLen);
        
        index += pointerLen;
        free(pointer);
        
        break;
      } else {
        scratch[index++] = buffer[(*offset)++];
      }
    }
    
    scratch[index++] = '\0';
    (*offset)++;
    
    *name = (char *)malloc(sizeof(char) * index);
    memcpy(*name, scratch, index);
    free(scratch);
    return 0;
  }

  int Resolver::assembleName(byte *buffer, unsigned int len, unsigned int *offset, char **name) {
    assembleName(buffer, len, offset, name, MDNS_MAX_NAME_LEN);
  }

  int Resolver::parseAnswer(byte *buffer, unsigned int len, unsigned int *offset, Answer *a) {
    char *assembled;
    assembleName(buffer, len, offset, &assembled);
    parseName(&a->name, assembled, strlen(assembled));
    free(assembled);

    a->type = (buffer[(*offset)++] << 8) + buffer[(*offset)++];
    
    a->aclass = buffer[(*offset)++];
    a->cacheflush = buffer[(*offset)++];

    a->ttl = (buffer[(*offset)++] << 24) + (buffer[(*offset)++] << 16) + (buffer[(*offset)++] << 8) + buffer[(*offset)++];
    a->len = (buffer[(*offset)++] << 8) + buffer[(*offset)++];

    if(a->type == 0x05) {
      unsigned int dataOffset = (*offset);
      (*offset) += a->len;
      
      assembleName(buffer, len, &dataOffset, &assembled, a->len);
      parseName((char **)&a->data, assembled, strlen(assembled));
      free(assembled);
      a->len = strlen((char *)a->data);
    } else {
      a->data = (byte *)malloc(sizeof(byte) * a->len);  
      memcpy(a->data, buffer + (*offset), a->len);
      (*offset) += a->len;
    }
    
    return 0;
  }
  
  void Resolver::parsePacket(byte *buffer, unsigned int len) {
    if((buffer[2] & 0b10000000) != 0b10000000) {
      // Not an answer packet
      return;
    }
    
    bool truncated = buffer[2] & 0b00000010;
    
    if (buffer[3] & 0b00001111) {
      Serial.println("None-zero response code");
      // Non-zero response code - Error
      return;
    }

    unsigned int answerCount = (buffer[6] << 8) + buffer[7];
    Serial.print("Answer Count: ");
    Serial.println(answerCount);

    
    // For this library, we are only interested in packets that contain answers
    if(answerCount > 0) {
      unsigned int offset = 0;
      
      if(skipQuestions(buffer, len, &offset) != 0) {
        Serial.println("Error parsing questions\n");
        return;
      }

      Serial.println("\n");
      
      for(int i = 0; i < answerCount; i++) {
        Answer answer;
        answer.name = NULL;
        answer.data = NULL;
        
        if(parseAnswer(buffer, len, &offset, &answer) == 0) {
          Serial.print("Name: ");
          Serial.println(answer.name);

          Serial.print("Type: ");
          Serial.println(answer.type);
      
          if(answer.type == 0x01 || answer.type == 0x05) {    
            Serial.print("Data: ");
            for(int i = 0; i < answer.len; i++) {
              if(answer.type == 0x01) {
                Serial.print(answer.data[i]);
                if(i != answer.len - 1) {
                  Serial.print(".");
                }
              } else if(answer.type == 0x05) {
                Serial.print(answer.data[i]);
              }
            }
            Serial.println("\n");
          }
          
          free(answer.data);
          free(answer.name);
          return;
        } else {
          Serial.println("Error parsing packet");
          
          if(answer.data) {
            free(answer.data);
          }
          if(answer.name) {
            free(answer.name);
          }
        }
      }
    }
  }

  Response Resolver::buildResponse(const char *name) {
    int len = strlen(name);
    
    Response empty;
    empty.waiting = true;
    empty.timeout = MDNS_TIMEOUT;
    empty.ttl = 0;
    empty.ipAddress = IPAddress(0, 0, 0, 0);
    
    empty.name = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(empty.name, name, len);
    empty.name[len] = '\0';

    return empty;
  }

  Query Resolver::buildQuery(const char *name) {
    int len = strlen(name);

    Query q;
    q.type = 0x01;
    q.qclass = 0x01;
    q.unicastResponse = false;
    q.valid = false;

    q.name = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(q.name, name, len);
    q.name[len] = '\0';

    return q;
  }

  void Resolver::broadcastQuery(Query q) {
    if (!_init) {
      _init = true;
      Serial.println("Initializing Multicast.");
      Udp.beginMulticast(WiFi.localIP(), IPAddress(224, 0, 0, 251), MDNS_TARGET_PORT);
    }
    
    int namelen = strlen(q.name);    
    int packetSize = namelen + 18;
    byte buffer[packetSize];

    for(int i = 0; i < packetSize; i++) {
      buffer[i] = 0;
    }

    buffer[4] = 0;
    buffer[5] = 1;

    int bufferlen = 12;

    int wordstart = 0, wordend = 0;
    do {
      if(q.name[wordend] == '.' || q.name[wordend] == '\0') {
        const int wordlen = wordend - wordstart;
        buffer[bufferlen++] = (byte)wordlen;
        for(int i = wordstart; i < wordend; i++) {
          buffer[bufferlen++] = q.name[i];
        }
        wordend++;
        wordstart = wordend;
      }
      
      wordend++;
    } while(q.name[wordstart] != '\0');
    
    buffer[bufferlen++] = '\0';
    buffer[bufferlen++] = (q.type & 0xFF00) >> 8;
    buffer[bufferlen++] = q.type & 0xFF;

    unsigned int qclass = 0;
    if(q.unicastResponse) {
      qclass = 0b1000000000000000;
    }

    qclass += q.qclass;
    
    buffer[bufferlen++] = (qclass & 0xFF00) >> 8;
    buffer[bufferlen++] = qclass & 0xFF;

    Udp.begin(MDNS_SOURCE_PORT);
    Udp.beginPacketMulticast(IPAddress(224, 0, 0, 251), MDNS_TARGET_PORT, WiFi.localIP(), MDNS_TTL);
    Udp.write(buffer, bufferlen);
    Udp.endPacket();
  }

  Response Resolver::query(const char *name) {
    int cachedIndex = search(name);
    if(cachedIndex == -1) {
      Response response = buildResponse(name);
      Query q = buildQuery(name);
      insert(response);
      broadcastQuery(q);
      return response;
    } else {
      return _cache[cachedIndex];
    }
  }

  void Resolver::expire() {
    // Check if it's been a second since the last sweep
    long now = millis();
    if(now - _lastSweep > 1000) {
      _lastSweep = now;

      for(int i = 0; i < _cacheCount; i++) {
        if(_cache[i].waiting) {
          _cache[i].timeout--;

          if(_cache[i].timeout <= 0) {
            remove(i);
          }
        } else {
          _cache[i].ttl--;
          
          if(_cache[i].ttl <= 0) {
            remove(i);
          }
        }
      }

      if(_cacheCount == 0) {
        Serial.println("Cache empty\n");
      }

      for(int i = 0; i < _cacheCount; i++) {
        Serial.print("Slot: ");
        Serial.println(i);

        Serial.print("Name: ");
        Serial.println(_cache[i].name);
        
        if(_cache[i].waiting) {
          Serial.println("Waiting: true");
          Serial.println("Timeout: ");
          Serial.println(_cache[i].timeout);
        } else {
          Serial.println("Waiting: false");
          Serial.println("TTL: ");
          Serial.println(_cache[i].ttl);
        }
      }
    }
  }

  void Resolver::insert(Response response) {
    if(_cacheCount < MDNS_RESOLVER_MAX_CACHE) {
      _cache[_cacheCount++] = response;
    } else {
      printf("Cache full\n");
    }
  }

  void Resolver::remove(int index) {
    free(_cache[index].name);
    _cache[index] = _cache[_cacheCount - 1];
    _cacheCount--;
  }

  int Resolver::search(const char *name) {
    for(int i = 0; i < _cacheCount; i++) {
      if(strcmp(_cache[i].name, name) == 0) {
        return i;
      }
    }
  }

  void Resolver::listen() {
    if (!_init) {
      _init = true;
      Serial.println("Initializing Multicast.");
      Udp.beginMulticast(WiFi.localIP(), IPAddress(224, 0, 0, 251), MDNS_TARGET_PORT);
    }
    unsigned int len = Udp.parsePacket();

    if(len > 0) {
      byte *buffer = (byte *)malloc(sizeof(byte) * len);
      Udp.read(buffer, len);
      parsePacket(buffer, len);
      free(buffer);
    }
  }
};
