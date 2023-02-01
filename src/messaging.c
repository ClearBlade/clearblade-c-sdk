#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "MQTTAsync.h"
#include "util.h"
#include "messaging.h"

MQTTAsync mqttClient = NULL;
int finished = 0;
char *clientID = "";
int qos = 0;


void onConnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("Connect failed, rc %d\n", response ? response->code : 0);
}


void connLost(void *context, char *cause) {
	printf("\nConnection lost\n");
	printf("Cause: %s\n", cause);
	printf("Reconnecting\n");
	connectToMQTT(clientID, qos, NULL, NULL);
}


void connectToMQTTAdvanced(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause), bool autoReconnect) {
	connectToMQTTInternal(clientId, qualityOfService, mqttOnConnect, messageArrivedCallback, onConnLostCallback, autoReconnect, NULL);
}

void connectToMQTT(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message)) {
	connectToMQTTAdvanced(clientId, qualityOfService, mqttOnConnect, messageArrivedCallback, NULL, false);
}

void connectToMQTTWithSSLOptions(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause), bool autoReconnect, struct MQTTSSLOptions *sslOptions) {
	connectToMQTTInternal(clientId, qualityOfService, mqttOnConnect, messageArrivedCallback, onConnLostCallback, autoReconnect, sslOptions);
}

void connectToMQTTInternal(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause), bool autoReconnect, struct MQTTSSLOptions *sslOptions) {
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

	rc = MQTTAsync_create(&client, messagingurl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if(rc != MQTTASYNC_SUCCESS) {
		printf("Failed to create MQTT client. Error code is %d\n", rc);
		return;
	}
	if(client == NULL) {
		printf("MQTT client creation failure. client is NULL\n");
		return;
	}
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
	
	// Note URL contains protocol before first colon (e.g. tcp://platform.clearblade.com:1883)
	char *addrOfFirstColon;
	addrOfFirstColon = strchr(messagingurl,':');
	int relLocOfFirstColon = (addrOfFirstColon - messagingurl) * sizeof(char);
    
	// Allocate array for protocol and retrieve it from messagingurl; add null at end
	char protocol[relLocOfFirstColon + 1];
	strncpy(protocol, messagingurl, relLocOfFirstColon);
	protocol[relLocOfFirstColon] = '\0';

	// If protocol is 'ssl' TLS is desired
	if (strcmp(protocol, "ssl") == 0) {
		MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
		conn_opts.ssl = &ssl_opts;
		setSSLOptions(&conn_opts, sslOptions);
	};

  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("Failed to start connect, return code %d\n", rc);
	MQTTAsync_destroy(&client);
    return;
  }

	while(finished == 0) {

	}

	mqttClient = client;
	finished = 0;
}

void setSSLOptions(MQTTAsync_connectOptions* connOpts, struct MQTTSSLOptions* sslOptions) {
	if (sslOptions == NULL) {
		return;
	}
	if (sslOptions->keyStore != NULL) {
		connOpts->ssl->keyStore = sslOptions->keyStore;
	}
	if (sslOptions->trustStore != NULL) {
		connOpts->ssl->trustStore = sslOptions->trustStore;
	}
}


void onSubscribe(void* context, MQTTAsync_successData* response) {
	printf("Subscribe succeeded\n");
	finished = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("Subscribe failed, rc %d\n", response ? response->code : 0);
	finished = 1;
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

	while(finished == 0) {

	}

	finished = 0;
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


void onDisconnect(void* context, MQTTAsync_successData* response) {
	printf("Successful disconnection\n");
	finished = 1;
}


void disconnectMQTTClient() {
	if(mqttClient == NULL) {
		printf("You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to disconnect\n");
		exit(-1);
	}

	int rc;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.onSuccess = onDisconnect;

	if((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start disconnect, return code %d\n", rc);
	        return;
	}
	MQTTAsync_destroy(&mqttClient);
	while (finished == 0) {

	}

	mqttClient = NULL;
	finished = 0;
}
