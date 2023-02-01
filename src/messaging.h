#ifndef _messaging_h
#define _messaging_h

#include "MQTTAsync.h"

struct MQTTSSLOptions {
	char *trustStore;
	char *keyStore;
};

void connectToMQTT(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message));

void connectToMQTTAdvanced(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause), bool autoReconnect);

void connectToMQTTWithSSLOptions(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause), bool autoReconnect, struct MQTTSSLOptions *sslOptions);

void subscribeToTopic(char *topic, int qos);

void publishMessage(char *message, char *topic, int qos, int retained);

void unsubscribeFromTopic(char *topic);

void disconnectMQTTClient();

#endif
