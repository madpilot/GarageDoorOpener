#include "MQTT.h"

long lastPubSubConnectionAttempt = 0;

void PubSubSetup(PubSubClient *pubSubClient, MQTT_CALLBACK_SIGNATURE) {
  pubSubClient->setServer("192.168.0.1", 1883);
  pubSubClient->setCallback(callback);
}

boolean PubSubConnect(PubSubClient *pubSubClient) {
  Serial.print("Connecting to MQTT server...");

  if(!pubSubClient->connect("garage")) {
    Serial.println("\nCouldn't connect to MQTT server. Will try again in 5 seconds.");
    return false;
  }

  if(!pubSubClient->subscribe(COMMAND_TOPIC, QOS_LEVEL)) {
    Serial.print("\nUnable to subscribe to ");
    Serial.println(COMMAND_TOPIC);
    pubSubClient->disconnect();
    return false;
  }

  Serial.println(" Connected.");
  return true;
}

void PubSubLoop(PubSubClient *pubSubClient) {
  if(!pubSubClient->connected()) {
    long now = millis();

    if(now - lastPubSubConnectionAttempt > 5000) {
      lastPubSubConnectionAttempt = now;
      if(PubSubConnect(pubSubClient)) {
        lastPubSubConnectionAttempt = 0;
      }
    }
  } else {
    pubSubClient->loop();
  }
}
