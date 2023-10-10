#ifndef _messaging_h
#define _messaging_h

#include "MQTTAsync.h"

typedef struct {
  int keepAliveInterval;
  bool cleanSession;
  int maxInFlight;
  int connectTimeout;
  int retryInterval;
	bool automaticReconnect;
	int minRetryInterval;
	int maxRetryInterval;
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;
	MQTTAsync_willOptions* will;

} CbMqttConnectOptions;

typedef struct {
  int timeout;
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;

} CbMqttDisconnectOptions;


CbMqttConnectOptions* getDefaultMQTTConnectOptions();

void connectToMQTT(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message));

void connectToMQTTAdvanced(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause), bool autoReconnect);

void connectCbMQTT(void* context, char *clientId, CbMqttConnectOptions *options, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause));

void subscribeToTopic(char *topic, int qos);

void publishMessage(char *message, char *topic, int qos, int retained);

void unsubscribeFromTopic(char *topic);

void disconnectMQTTClient();

void disconnectCbMQTT(void* context, CbMqttDisconnectOptions *options);

#endif
