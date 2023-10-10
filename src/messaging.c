#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "MQTTAsync.h"
#include "util.h"
#include "messaging.h"

MQTTAsync mqttClient = NULL;
//int finished = 0;
char *clientID = "";
int qos = 0;

CbMqttConnectOptions getDefaultConnectOptions() {
	CbMqttConnectOptions options;

	//These defaults are inline with what paho uses in the MQTTAsync_connectOptions_initializer
	options.keepAliveInterval = 60;
	options.cleanSession = true;
	options.maxInFlight = 65535;
	options.connectTimeout = 30;
	options.retryInterval = 0;
	options.automaticReconnect = false;
	options.minRetryInterval = 1;
	options.maxRetryInterval = 60;

	return options;
}

CbMqttDisconnectOptions getDefaultDisconnectOptions() {
	CbMqttDisconnectOptions options;

	//These defaults are inline with what paho uses in the MQTTAsync_disconnectOptions_initializer
	options.timeout = 0;
	options.onSuccess = NULL;
	options.onFailure = NULL;

	return options;
}

void onConnectSuccess(void *context, MQTTAsync_successData *response) {
	printf("MQTT Connection successful\n");
}

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("MQTT Connect failed, rc %d\n", response ? response->code : 0);
}

void connLost(void *context, char *cause) {
	printf("\nMQTT Connection lost\n");
	printf("Cause: %s\n", cause);
}


void connectToMQTTAdvanced(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause), bool autoReconnect) {
	
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
	if(onConnLostCallback == NULL) {
		MQTTAsync_setCallbacks(client, NULL, connLost, messageArrivedCallback, NULL);
	} else {
		MQTTAsync_setCallbacks(client, NULL, onConnLostCallback, messageArrivedCallback, NULL);
	}

	if(autoReconnect) {
		conn_opts.automaticReconnect = 1;
	}
	conn_opts.keepAliveInterval = 60;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = mqttOnConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
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

	//while(finished == 0) {

	//}

	mqttClient = client;
	//finished = 0;
}

void connectToMQTT(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message)) {

	connectToMQTTAdvanced(clientId, qualityOfService, mqttOnConnect, messageArrivedCallback, NULL, false);
}


void connectCbMQTT(void* context, char *clientId, CbMqttConnectOptions *options, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause)) {

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
	if(onConnLostCallback == NULL) {
		MQTTAsync_setCallbacks(client, context, connLost, messageArrivedCallback, NULL);
	} else {
		MQTTAsync_setCallbacks(client, context, onConnLostCallback, messageArrivedCallback, NULL);
	}

	conn_opts.keepAliveInterval = options->keepAliveInterval;
	conn_opts.cleansession = options->cleanSession;
	conn_opts.maxInflight = options->maxInFlight;
	conn_opts.connectTimeout = options->connectTimeout;
	conn_opts.retryInterval = options->retryInterval;
	conn_opts.automaticReconnect = options->automaticReconnect;
	conn_opts.minRetryInterval = options->minRetryInterval;
	conn_opts.maxRetryInterval = options->maxRetryInterval;

	if (mqttOnConnect == NULL) {
		conn_opts.onSuccess = onConnectSuccess;
	} else {
		conn_opts.onSuccess = mqttOnConnect;
	}

	if (options->onFailure == NULL) {
		conn_opts.onFailure = onConnectFailure;
	} else {
		conn_opts.onFailure = options->onFailure;
	}

	conn_opts.context = context;
	conn_opts.username = username;
	conn_opts.password = password;

	// If protocol is 'ssl' TLS is desired
	if (strstr(messagingurl, "ssl://") == messagingurl) {
		MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
		conn_opts.ssl = &ssl_opts;
	};

	//Add the last will and testament options
	if(options->will != NULL) {
		conn_opts.will = options->will;
	}

	//Try to connect to the MQTT client
  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("Failed to start connect, return code %d\n", rc);
		MQTTAsync_destroy(&client);
    return;
  }
}

void onSubscribe(void* context, MQTTAsync_successData* response) {
	printf("Subscribe succeeded\n");
	//finished = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("Subscribe failed, rc %d\n", response ? response->code : 0);
	//finished = 1;
}


void subscribeToTopic(char *topic, int qos) {
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to subscribe\n");
		return;
	}

	int rc;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;

	if((rc = MQTTAsync_subscribe(mqttClient, topic, qos, &opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start subscribeToTopic, return code %d\n", rc);
		return;
	}

	// while(finished == 0) {

	// }

	// finished = 0;
}


void publishMessage(char *message, char *topic, int qos, int retained) {
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to publish\n");
		return;
	}

	int rc;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	pubmsg.payload = message;
	pubmsg.payloadlen = strlen(message);
	pubmsg.qos = qos;
	pubmsg.retained = retained;

	if ((rc = MQTTAsync_sendMessage(mqttClient, topic, &pubmsg, NULL)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start publishMessage, return code %d\n", rc);
		return;
	}
}


void unsubscribeFromTopic(char *topic) {
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to unsubscribe\n");
		return;
	}

	int rc;
	if((rc = MQTTAsync_unsubscribe(mqttClient, topic, NULL)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start unsubscribe, return code %d\n", rc);
	        return;
	}
}


void onDisconnectSuccess(void* context, MQTTAsync_successData* response) {
	printf("MQTT disconnect success\n");
}

void onDisconnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("MQTT disconnect failed, rc %d\n", response ? response->code : 0);
}

void disconnectMQTTClient() {
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to disconnect\n");
		exit(-1);
	}

	int rc;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.onSuccess = onDisconnectSuccess;

	if((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start disconnect, return code %d\n", rc);
	        return;
	}
	MQTTAsync_destroy(&mqttClient);
	// while (finished == 0) {

	// }

	mqttClient = NULL;
	//finished = 0;
}

void disconnectCbMQTT(void* context, CbMqttDisconnectOptions *options) {
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to disconnect\n");
		exit(-1);
	}

	int rc;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	
	disc_opts.timeout = options->timeout;

	if (options->onSuccess == NULL) {
		disc_opts.onSuccess = onDisconnectSuccess;
	} else {
		disc_opts.onSuccess = options->onSuccess;
	}

	if (options->onFailure == NULL) {
		disc_opts.onFailure = onDisconnectFailure;
	} else {
		disc_opts.onFailure = options->onFailure;
	}

	if((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start disconnect, return code %d\n", rc);
	        return;
	}
	MQTTAsync_destroy(&mqttClient);
	mqttClient = NULL;
}
