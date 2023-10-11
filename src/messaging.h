#ifndef _messaging_h
#define _messaging_h

#include "MQTTAsync.h"

struct CbMqttCallbacks {
	MQTTAsync_onSuccess* onConnectSuccess;
	MQTTAsync_onFailure* onConnectFailure;
	MQTTAsync_connectionLost* onConnectionLost;
	MQTTAsync_onSuccess* onDisconnectSuccess;
	MQTTAsync_onFailure* onDisconnectFailure;
	MQTTAsync_messageArrived* messageArrived;
	MQTTAsync_onSuccess* onSubscribeSuccess;
	MQTTAsync_onFailure* onSubscribeFailure;
	MQTTAsync_onSuccess* onPublishSuccess;
	MQTTAsync_onFailure* onPublishFailure;
	MQTTAsync_onSuccess* onUnsubscribeSuccess;
	MQTTAsync_onFailure* onUnsubscribeFailure;
};

extern struct CbMqttCallbacks callbacks;

struct CbMqttConnectOptions {
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
};

struct CbMqttDisconnectOptions {
  int timeout;
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;
};

struct CbMqttResponseOptions {
  int timeout;
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;
};

struct CbMqttConnectOptions getDefaultCbMQTTConnectOptions();
struct CbMqttDisconnectOptions getDefaultCbMQTTDisconnectOptions();
struct CbMqttResponseOptions getDefaultCbMQTTResponseOptions();

void connectToMQTT(char *clientId, int qualityOfService, MQTTAsync_onSuccess *mqttOnConnect,
 		MQTTAsync_messageArrived* messageArrivedCallback);

void connectToMQTTAdvanced(char *clientId, int qualityOfService, MQTTAsync_onSuccess* mqttOnConnect,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback, bool autoReconnect);

void connectCbMQTT(void* context, char *clientId, struct CbMqttConnectOptions *options,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback);

void subscribeToTopic(char *topic, int qos);
void subscribeCbMQTT(void* context, char *topic, int qos, struct CbMqttResponseOptions* options);

void publishMessage(char *message, char *topic, int qos, int retained);
void publishCbMQTT(void* context, char *message, char *topic, int qos, int retained, struct CbMqttResponseOptions* options);

void unsubscribeFromTopic(char *topic);
void unsubscribeCbMQTT(void* context, char *topic, struct CbMqttResponseOptions* options);

void disconnectMQTTClient();
void disconnectCbMQTT(void* context, struct CbMqttDisconnectOptions *options);

#endif
