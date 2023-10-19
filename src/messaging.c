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

CbMqttCallbacks callbacks = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

CbMqttConnectOptions* getDefaultCbMQTTConnectOptions() {
	CbMqttConnectOptions* options = malloc(sizeof(CbMqttConnectOptions));

	//These defaults are inline with what paho uses in the MQTTAsync_connectOptions_initializer
	options->keepAliveInterval = 60;
	options->cleanSession = true;
	options->maxInFlight = 65535;
	options->connectTimeout = 30;
	options->retryInterval = 0;
	options->automaticReconnect = false;
	options->minRetryInterval = 1;
	options->maxRetryInterval = 60;
	options->onSuccess = NULL;
	options->onFailure = NULL;

	return options;
}

CbMqttDisconnectOptions* getDefaultCbMQTTDisconnectOptions() {
	CbMqttDisconnectOptions* options = malloc(sizeof(CbMqttDisconnectOptions));

	//These defaults are inline with what paho uses in the MQTTAsync_disconnectOptions_initializer
	options->timeout = 0;
	options->onSuccess = NULL;
	options->onFailure = NULL;

	return options;
}

CbMqttSubscribeOptions* getDefaultCbMQTTSubscribeOptions() {
	//These defaults are inline with what paho uses in the MQTTSubscribe_options_initializer
	CbMqttSubscribeOptions* options = malloc(sizeof(CbMqttSubscribeOptions));
	options->noLocal = 0;
	options->retainAsPublished = 0;
	options->retainHandling = 0;

	return options;
}

CbMqttResponseOptions* getDefaultCbMQTTResponseOptions() {
	CbMqttResponseOptions* options = malloc(sizeof(CbMqttResponseOptions));

	options->onSuccess = NULL;
	options->onFailure = NULL;
	options->subscribeOptions = getDefaultCbMQTTSubscribeOptions();

	return options;
}

void waitForOperation() {
	while(operationInProgress == true) {
	}
	operationInProgress = true;
}

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
	printf("C SDK - onMessageArrived\n");
	if (callbacks.messageArrived != NULL) {
		return callbacks.messageArrived(context, topicName, topicLen, message);
	} else {
		return 0;
	}
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

	if (callbacks.onConnectFailure != NULL) callbacks.onConnectFailure(context, response);
}

void onConnectionLost(void *context, char *cause) {
	printf("\nC SDK - onConnectionLost: MQTT Connection lost, Cause: %s\n", cause);
	if (callbacks.onConnectionLost != NULL) callbacks.onConnectionLost(context, cause);
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
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	if (strstr(messagingurl, "ssl://") == messagingurl) {
		conn_opts.ssl = &ssl_opts;
	};

  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("Failed to start connect, return code %d\n", rc);
		MQTTAsync_destroy(&client);
    return;
  }

	waitForOperation();
	mqttClient = client;
}

void connectCbMQTT(void* context, char *clientId, CbMqttConnectOptions *options,
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

	//Add the last will and testament options
	// if(options->will != NULL) {
	// 	conn_opts.will = options->will;
	// }

	conn_opts.username = username;
	conn_opts.password = password;
	conn_opts.connectTimeout = options->connectTimeout;
	conn_opts.retryInterval = options->retryInterval;

		// If protocol is 'ssl' TLS is desired
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	if (strstr(messagingurl, "ssl://") == messagingurl) {
		conn_opts.ssl = &ssl_opts;
	};
	
	if (context != NULL) {
		conn_opts.context = context;
	}
	
	conn_opts.automaticReconnect = options->automaticReconnect;
	conn_opts.minRetryInterval = options->minRetryInterval;
	conn_opts.maxRetryInterval = options->maxRetryInterval;

	//Try to connect to the MQTT client
	int rc;
  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("C SDK - Failed to connect to MQTT, return code %d\n", rc);

		//invoke the callback so that the client is aware of the failure
		MQTTAsync_failureData* failureData = malloc(sizeof(MQTTAsync_failureData));
		failureData->token = 0;
		failureData->code = rc;
		failureData->message=(const char*)"Failed to connect to MQTT";

		onConnectFailure(context, failureData);

		MQTTAsync_destroy(&client);
    return;
  }
	
	waitForOperation();
	mqttClient = client;
}

void onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - onSubscribeSuccess\n");
	if (callbacks.onSubscribeSuccess != NULL) callbacks.onSubscribeSuccess(context, response);
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - onSubscribeFailure\n");
	printf("C SDK - Subscribe failed, rc %d\n", response ? response->code : 0);
	if (callbacks.onSubscribeFailure != NULL) callbacks.onSubscribeFailure(context, response);
}

void subscribeCbMQTT(void* context, char *topic, int qos, CbMqttResponseOptions* options) {
	printf("C SDK - cbMQTTSubscribe\n");

	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please invoke connectCbMQTT() first and then try to subscribe\n");
		return;
	}

	MQTTAsync_responseOptions responseOpts = MQTTAsync_responseOptions_initializer;

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

	responseOpts.onSuccess = onSubscribeSuccess;
	responseOpts.onFailure = onSubscribeFailure;

	//subscribe options
	if (options->subscribeOptions->noLocal) responseOpts.subscribeOptions.noLocal = options->subscribeOptions->noLocal;
	if (options->subscribeOptions->retainAsPublished) responseOpts.subscribeOptions.retainAsPublished = options->subscribeOptions->retainAsPublished;
	if (options->subscribeOptions->retainHandling) responseOpts.subscribeOptions.retainHandling = options->subscribeOptions->retainHandling;

	if (context != NULL) responseOpts.context = context;

	printf("C SDK - Invoking MQTTAsync_subscribe\n");

	int rc;
	if((rc = MQTTAsync_subscribe(mqttClient, topic, qos, &responseOpts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start subscribeToTopic, return code %d\n", rc);
		return;
	}
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
	printf("C SDK - Publish failed, rc %d\n", response ? response->code : 0);
	operationInProgress = false;

	if (callbacks.onPublishFailure != NULL) callbacks.onPublishFailure(context, response);
}

void publishCbMQTT(void* context, char *message, char *topic, int qos, int retained, CbMqttResponseOptions* options) {
	printf("C SDK - cbMQTTSubscribe\n");
	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please invoke connectCbMQTT() first and then try to publish\n");
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

	if (callbacks.onUnsubscribeSuccess != NULL) callbacks.onUnsubscribeSuccess(context, response);
}

void onUnsubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - onUnsubscribeFailure\n");
	printf("C SDK - Unsubscribe failed, rc %d\n", response ? response->code : 0);

	if (callbacks.onUnsubscribeFailure != NULL) callbacks.onUnsubscribeFailure(context, response);
}

void unsubscribeCbMQTT(void* context, char *topic, CbMqttResponseOptions* options) {
	printf("C SDK - unsubscribeCbMQTT\n");
	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please call connectCbMQTT() first and then try to unsubscribe\n");
		return;
	}

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

	int rc;
	if((rc = MQTTAsync_unsubscribe(mqttClient, topic, &opts)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start unsubscribe, return code %d\n", rc);
	        return;
	}
}

void unsubscribeFromTopic(char *topic) {
	printf("C SDK - unsubscribeFromTopic\n");
	unsubscribeCbMQTT(NULL, topic, NULL);
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

void disconnectMQTTClient() {
	printf("C SDK - disconnectMQTTClient\n");
	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to disconnect\n");
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

void disconnectCbMQTT(void* context, CbMqttDisconnectOptions *options) {
	printf("C SDK - disconnectCbMQTT\n");

	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please call connectCbMQTT() first and then try to disconnect\n");
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
	  printf("C SDK - Failed to start disconnect, return code %d\n", rc);
	  return;
	}

	//waitForOperation();
	//MQTTAsync_destroy(&mqttClient);
	mqttClient = NULL;
}
