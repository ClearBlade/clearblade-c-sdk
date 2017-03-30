#ifndef _messaging_h
#define _messaging_h

#include "MQTTAsync.h"

void connectToMQTT(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message));

void connectToMQTTAdvanced(char *clientID, int qos, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
                  int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
                  void (*onConnLostCallback)(void *context, char *cause));

void subscribeToTopic(char *topic, int qos);

void publishMessage(char *message, char *topic, int qos, int retained);

void unsubscribeFromTopic(char *topic);

void disconnectMQTTClient();

#endif
