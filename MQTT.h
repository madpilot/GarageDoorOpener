#include "Arduino.h"
#include <PubSubClient.h>

#define STATE_TOPIC "home-assistant/garage"
#define COMMAND_TOPIC "home-assistant/garage/set"

#define OPEN_COMMAND "OPEN"
#define CLOSE_COMMAND "CLOSE"

#define OPENED_PAYLOAD "OPENED"
#define CLOSED_PAYLOAD "CLOSED"
#define OPENING_PAYLOAD "OPENING"
#define CLOSING_PAYLOAD "CLOSING"

#define QOS_LEVEL 0

void PubSubSetup(PubSubClient *pubSubClient, MQTT_CALLBACK_SIGNATURE);
boolean PubSubConnect(PubSubClient *pubSubClient);
void PubSubLoop(PubSubClient *pubSubClient);
