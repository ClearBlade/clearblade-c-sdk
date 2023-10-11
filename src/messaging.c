#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "MQTTAsync.h"
#include "util.h"
#include "messaging.h"

MQTTAsync mqttClient = NULL;
bool operationInProgress = false;
char *clientID = "";
int qos = 0;

struct CbMqttCallbacks callbacks = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

struct CbMqttConnectOptions getDefaultCbMQTTConnectOptions() {
	struct CbMqttConnectOptions options;

	//These defaults are inline with what paho uses in the MQTTAsync_connectOptions_initializer
	options.keepAliveInterval = 60;
	options.cleanSession = true;
	options.maxInFlight = 65535;
	options.connectTimeout = 30;
	options.retryInterval = 0;
	options.automaticReconnect = false;
	options.minRetryInterval = 1;
	options.maxRetryInterval = 60;
	options.onSuccess = NULL;
	options.onFailure = NULL;

	return options;
}

struct CbMqttDisconnectOptions getDefaultCbMQTTDisconnectOptions() {
	struct CbMqttDisconnectOptions options;

	//These defaults are inline with what paho uses in the MQTTAsync_disconnectOptions_initializer
	options.timeout = 0;
	options.onSuccess = NULL;
	options.onFailure = NULL;

	return options;
}

struct CbMqttResponseOptions getDefaultCbMQTTResponseOptions() {
	struct CbMqttResponseOptions options;

	options.onSuccess = NULL;
	options.onFailure = NULL;

	return options;
}

void waitForOperation() {
	while(operationInProgress == true) {
	}
	operationInProgress = true;
}

void onConnectSuccess(void *context, MQTTAsync_successData *response) {
	printf("C SDK - onConnectSuccess\n");

	operationInProgress = false;

	if (callbacks.onConnectSuccess != NULL) {
		callbacks.onConnectSuccess(context, response);
	}
}

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - onConnectFailure\n");
	operationInProgress = false;

	if (callbacks.onDisconnectFailure != NULL) callbacks.onDisconnectFailure(context, response);
}

void onConnectionLost(void *context, char *cause) {
	printf("\nC SDK - onConnectionLost: MQTT Connection lost\n");
	printf("Cause: %s\n", cause);

	if (callbacks.onConnectionLost != NULL) callbacks.onConnectionLost(context, cause);
}

void onDisconnectSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - onDisconnectSuccess\n");
	operationInProgress = false;

	if (callbacks.onDisconnectSuccess != NULL) callbacks.onDisconnectSuccess(context, response);
}

void onDisconnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - onDisconnectFailure\n");
	printf("MQTT disconnect failed, rc %d\n", response ? response->code : 0);
	operationInProgress = false;

	if (callbacks.onDisconnectFailure != NULL) callbacks.onDisconnectFailure(context, response);
}

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
	printf("C SDK - onMessageArrived\n");
	if (callbacks.messageArrived != NULL) {
		return callbacks.messageArrived(context, topicName, topicLen, message);
	} else {
		return 0;
	}
}

void connectToMQTT(char *clientId, int qualityOfService, MQTTAsync_onSuccess* mqttOnConnect, MQTTAsync_messageArrived* messageArrivedCallback) {
	printf("C SDK - connectToMQTT\n");
	connectToMQTTAdvanced(clientId, qualityOfService, mqttOnConnect, messageArrivedCallback, NULL, false);
}

void connectToMQTTAdvanced(char *clientId, int qualityOfService, MQTTAsync_onSuccess* mqttOnConnect,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback, bool autoReconnect) {
	
	printf("C SDK - connectToMQTTAdvanced\n");

	if (getUserToken() == NULL && getDeviceToken() == NULL) {
		fprintf(stderr, "connectToMQTT called with unset auth token\n");
		return;
	}
	if (qualityOfService < 0 || qualityOfService > 2) {
		fprintf(stderr, "Quality of service cannot be less than 0 or greater than 2\n");
		return;
	}

	const char *username = getUserToken();
	if (username == NULL) {
		username = getDeviceToken();
	}
	const char *password = getSystemKey();
	char *messagingurl = getMessagingURL();

	clientID = clientId;
	qos = qualityOfService;

	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	MQTTAsync_create(&client, messagingurl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	
	//Set callbacks
	if (mqttOnConnect == NULL) {
		printf("mqttOnConnect NULL\n");
		callbacks.onConnectSuccess = NULL;
	} else {
		callbacks.onConnectSuccess = mqttOnConnect;
	}
	conn_opts.onSuccess = onConnectSuccess;

	if(onConnLostCallback == NULL) {
		callbacks.onConnectionLost = NULL;
	} else {
		callbacks.onConnectionLost = onConnLostCallback;
	}

	if(messageArrivedCallback == NULL) {
		callbacks.messageArrived = NULL;
	} else {
		callbacks.messageArrived = messageArrivedCallback;
	}
	MQTTAsync_setCallbacks(client, NULL, onConnectionLost, onMessageArrived, NULL);


	if(autoReconnect) {
		conn_opts.automaticReconnect = 1;
	}
	conn_opts.keepAliveInterval = 60;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnectSuccess;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.username = username;
	conn_opts.password = password;

	// If protocol is 'ssl' TLS is desired
	if (strstr(messagingurl, "ssl://") == messagingurl) {
		MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
		conn_opts.ssl = &ssl_opts;
	};

  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("Failed to start connect, return code %d\n", rc);
		MQTTAsync_destroy(&client);
    return;
  }

	printf("Waiting for operation\n");
	waitForOperation();
	printf("Done waiting\n");
	mqttClient = client;
}

void connectCbMQTT(void* context, char *clientId, struct CbMqttConnectOptions *options,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback) {

	printf("C SDK - connectCbMQTT\n");

	//Use https://github.com/hivemq-cloud/paho-C-mqtt-client-example/blob/master/main.c as sample code
	if (getUserToken() == NULL && getDeviceToken() == NULL) {
		fprintf(stderr, "connectToMQTT called with unset auth token\n");
		return;
	}

	const char *username = getUserToken();
	if (username == NULL) {
		username = getDeviceToken();
	}
	const char *password = getSystemKey();
	char *messagingurl = getMessagingURL();

	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	MQTTAsync_create(&client, messagingurl, clientId, MQTTCLIENT_PERSISTENCE_NONE, context);

	//Set callbacks
	if (options->onSuccess == NULL) {
		callbacks.onConnectSuccess = NULL;
	} else {
		callbacks.onConnectSuccess = options->onSuccess;
	}
	conn_opts.onSuccess = onConnectSuccess;

	if (options->onFailure == NULL) {
		callbacks.onConnectFailure = NULL;
	} else {
		callbacks.onConnectFailure = options->onFailure;
	}
	conn_opts.onFailure = onConnectFailure;

	if(onConnLostCallback == NULL) {
		callbacks.onConnectionLost = NULL;
	} else {
		callbacks.onConnectionLost = onConnLostCallback;
	}

	if(messageArrivedCallback == NULL) {
		callbacks.messageArrived = NULL;
	} else {
		callbacks.messageArrived = messageArrivedCallback;
	}

	MQTTAsync_setCallbacks(client, context, onConnectionLost, onMessageArrived, NULL);

	conn_opts.keepAliveInterval = options->keepAliveInterval;
	conn_opts.cleansession = options->cleanSession;
	conn_opts.maxInflight = options->maxInFlight;
	conn_opts.connectTimeout = options->connectTimeout;
	conn_opts.retryInterval = options->retryInterval;
	conn_opts.automaticReconnect = options->automaticReconnect;
	conn_opts.minRetryInterval = options->minRetryInterval;
	conn_opts.maxRetryInterval = options->maxRetryInterval;

	if (context != NULL) {
		conn_opts.context = context;
	}

	conn_opts.username = username;
	conn_opts.password = password;

	// If protocol is 'ssl' TLS is desired
	if (strstr(messagingurl, "ssl://") == messagingurl) {
		MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
		conn_opts.ssl = &ssl_opts;
	};

	//Add the last will and testament options
	// if(options->will != NULL) {
	// 	conn_opts.will = options->will;
	// }

	//Try to connect to the MQTT client
	printf("connectCbMQTT - Connecting to MQTT\n");
  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("Failed to start connect, return code %d\n", rc);
		MQTTAsync_destroy(&client);
    return;
  }

	mqttClient = client;
	
	printf("Waiting for operation\n");
	waitForOperation();
	printf("Done waiting\n");
}

void onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - onSubscribeSuccess\n");
	operationInProgress = false;

	if (callbacks.onSubscribeSuccess != NULL) callbacks.onSubscribeSuccess(context, response);
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - onSubscribeFailure\n");
	printf("Subscribe failed, rc %d\n", response ? response->code : 0);
	operationInProgress = false;

	if (callbacks.onSubscribeFailure != NULL) callbacks.onSubscribeFailure(context, response);
}

void subscribeCbMQTT(void* context, char *topic, int qos, struct CbMqttResponseOptions* options) {
	printf("C SDK - cbMQTTSubscribe\n");

	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to subscribe\n");
		return;
	}

	int rc;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

	//Set callbacks
	if (options != NULL) {
		if (options->onSuccess == NULL) {
			callbacks.onSubscribeSuccess = NULL;
		} else {
			callbacks.onSubscribeSuccess = options->onSuccess;
		}

		if (options->onFailure == NULL) {
			callbacks.onSubscribeFailure = NULL;
		} else {
			callbacks.onSubscribeFailure = options->onFailure;
		}
	}

	opts.onSuccess = onSubscribeSuccess;
	opts.onFailure = onSubscribeFailure;

	if (context != NULL) opts.context = context;

	if((rc = MQTTAsync_subscribe(mqttClient, topic, qos, &opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start subscribeToTopic, return code %d\n", rc);
		return;
	}

	waitForOperation();
}

void subscribeToTopic(char *topic, int qos) {
	printf("C SDK - subscribeToTopic\n");

	subscribeCbMQTT(NULL, topic, qos, NULL);
}

void onPublishSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - onPublishSuccess\n");
	operationInProgress = false;

	if (callbacks.onPublishSuccess != NULL) callbacks.onPublishSuccess(context, response);
}

void onPublishFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - onPublishFailure\n");
	printf("Publish failed, rc %d\n", response ? response->code : 0);
	operationInProgress = false;

	if (callbacks.onPublishFailure != NULL) callbacks.onPublishFailure(context, response);
}

void publishCbMQTT(void* context, char *message, char *topic, int qos, int retained, struct CbMqttResponseOptions* options) {
	printf("C SDK - cbMQTTSubscribe\n");
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to publish\n");
		return;
	}

	int rc;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	
	pubmsg.payload = message;
	pubmsg.payloadlen = strlen(message);
	pubmsg.qos = qos;
	pubmsg.retained = retained;

	//Set callbacks
	if (options != NULL) {
		if (options->onSuccess == NULL) {
			callbacks.onPublishSuccess = NULL;
		} else {
			callbacks.onPublishSuccess = options->onSuccess;
		}

		if (options->onFailure == NULL) {
			callbacks.onPublishFailure = NULL;
		} else {
			callbacks.onPublishFailure = options->onFailure;
		}
	}

	opts.onSuccess = onPublishSuccess;
	opts.onFailure = onPublishFailure;

	if (context != NULL) opts.context = context;

	if ((rc = MQTTAsync_sendMessage(mqttClient, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start publishMessage, return code %d\n", rc);
		return;
	}
}

void publishMessage(char *message, char *topic, int qos, int retained) {
	printf("C SDK - publishMessage\n");
	publishCbMQTT(NULL, message, topic, qos, retained, NULL);
}

void onUnsubscribeSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - onUnsubscribeSuccess\n");
	operationInProgress = false;

	if (callbacks.onUnsubscribeSuccess != NULL) callbacks.onUnsubscribeSuccess(context, response);
}

void onUnsubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - onUnsubscribeFailure\n");
	printf("Unsubscribe failed, rc %d\n", response ? response->code : 0);
	operationInProgress = false;

	if (callbacks.onUnsubscribeFailure != NULL) callbacks.onUnsubscribeFailure(context, response);
}

void unsubscribeCbMQTT(void* context, char *topic, struct CbMqttResponseOptions* options) {
	printf("C SDK - unsubscribeCbMQTT\n");
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to unsubscribe\n");
		return;
	}

	int rc;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

	//Set callbacks
	if (options != NULL) {
		if (options->onSuccess == NULL) {
			callbacks.onUnsubscribeSuccess = NULL;
		} else {
			callbacks.onUnsubscribeSuccess = options->onSuccess;
		}

		if (options->onFailure == NULL) {
			callbacks.onUnsubscribeFailure = NULL;
		} else {
			callbacks.onUnsubscribeFailure = options->onFailure;
		}
	}

	opts.onSuccess = onUnsubscribeSuccess;
	opts.onFailure = onUnsubscribeFailure;

	if (context != NULL) opts.context = context;

	if((rc = MQTTAsync_unsubscribe(mqttClient, topic, &opts)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start unsubscribe, return code %d\n", rc);
	        return;
	}

	waitForOperation();
}

void unsubscribeFromTopic(char *topic) {
	printf("C SDK - unsubscribeFromTopic\n");
	unsubscribeCbMQTT(NULL, topic, NULL);
}

void disconnectMQTTClient() {
	printf("C SDK - disconnectMQTTClient\n");
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to disconnect\n");
		exit(-1);
	}

	int rc;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.onSuccess = onDisconnectSuccess;
	disc_opts.onFailure = onDisconnectFailure;

	if((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start disconnect, return code %d\n", rc);
	        return;
	}

	//waitForOperation();
	//MQTTAsync_destroy(&mqttClient);
	mqttClient = NULL;
}

void disconnectCbMQTT(void* context, struct CbMqttDisconnectOptions *options) {
	printf("C SDK - disconnectCbMQTT\n");

	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectCbMQTT() first and then try to disconnect\n");
		exit(-1);
	}

	int rc;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.timeout = options->timeout;

	if (options->onSuccess == NULL) {
		callbacks.onDisconnectSuccess = NULL;
	} else {
		callbacks.onDisconnectSuccess = options->onSuccess;
	}
	disc_opts.onSuccess = onDisconnectSuccess;

	if (options->onFailure == NULL) {
		callbacks.onDisconnectFailure = NULL;
	} else {
		callbacks.onDisconnectFailure = options->onFailure;
	}
	disc_opts.onFailure = onDisconnectFailure;
	disc_opts.context = context;

	operationInProgress = true;
	
	if((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
	  printf("Failed to start disconnect, return code %d\n", rc);
	  return;
	}

	//waitForOperation();
	//MQTTAsync_destroy(&mqttClient);
	mqttClient = NULL;
}
