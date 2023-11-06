#ifndef _messaging_h
#define _messaging_h

#include "MQTTAsync.h"

typedef struct {
	MQTTAsync_connectionLost* onConnectionLost;
	MQTTAsync_messageArrived* messageArrived;
} CbMqttCallbacks;

extern CbMqttCallbacks callbacks;

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

typedef struct  {
  int noLocal;
	int retainAsPublished;
	int retainHandling;
} CbMqttSubscribeOptions;

typedef struct  {
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;
	CbMqttSubscribeOptions* subscribeOptions;
} CbMqttResponseOptions;

CbMqttConnectOptions* getDefaultCbMQTTConnectOptions();
CbMqttDisconnectOptions* getDefaultCbMQTTDisconnectOptions();
CbMqttResponseOptions* getDefaultCbMQTTResponseOptions();
CbMqttSubscribeOptions* getDefaultCbMQTTSubscribeOptions();

void connectToMQTT(char *clientId, int qualityOfService, MQTTAsync_onSuccess *mqttOnConnect,
 		MQTTAsync_messageArrived* messageArrivedCallback) __attribute__ ((deprecated));

void connectToMQTTAdvanced(char *clientId, int qualityOfService, MQTTAsync_onSuccess* mqttOnConnect,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback, bool autoReconnect) __attribute__ ((deprecated));

void connectCbMQTT(void* context, char *clientId, CbMqttConnectOptions *options,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback);

void subscribeToTopic(char *topic, int qos) __attribute__ ((deprecated));
void subscribeCbMQTT(void* context, char *topic, int qos, CbMqttResponseOptions* options);

void publishMessage(char *message, char *topic, int qos, int retained) __attribute__ ((deprecated));
void publishCbMQTT(void* context, char *message, char *topic, int qos, int retained, CbMqttResponseOptions* options);

void unsubscribeFromTopic(char *topic) __attribute__ ((deprecated));
void unsubscribeCbMQTT(void* context, char *topic, CbMqttResponseOptions* options);

void disconnectMQTTClient() __attribute__ ((deprecated));
void disconnectCbMQTT(void* context, CbMqttDisconnectOptions *options);

#endif
