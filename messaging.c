#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include "util.h"
#include "messaging.h"

int qos = 0;
bool isConnected = false;
bool dataArrived = false;
struct mosquitto *mosq = NULL;
void (*messagingCallback)(bool error, char *result) = NULL;

bool mosquittoloop() {
	mosquitto_loop(mosq, -1, 1);
	return dataArrived;
}

void setDataArrivedFlag() {
	dataArrived = false;
}

void runloop() {
	while (1) {
		bool flag = mosquittoloop();
		if (flag)
			break;
	}
	sleep(1);
	setDataArrivedFlag();
}

void connectCallback(struct mosquitto *mosq, void *userdata, int result) {
	isConnected = true;
	dataArrived = true;
	messagingCallback(false, "Mosquitto MQTT: Client Connected");
}

void subscribeCallback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) {
	dataArrived = true;
}

void messageArrivedCallback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
	dataArrived = true;
	printf("Topic: %s Message: %s\n", message->topic, message->payload);
}

void publishCallback(struct mosquitto *mosq, void *userdata, int result) {
	dataArrived = true;
	//printf("Mosquitto MQTT: Published Message\n");
}

void unsubscribeCallback(struct mosquitto *mosq, void *userdata, int result) {
	dataArrived = true;
}

void disconnectCallback(struct mosquitto *mosq, void *userdata, int result) {
	printf("Mosquitto MQTT: Client Disconnected\n");
}

void logCallback(struct mosquitto *mosq, void *obj, int level, const char *str) {
	printf("Mosquitto MQTT Log: %s\n", str);
}

void connect(char *host, int port, void mqttConnectCallback(bool error, char *result)) {
	int connectionResult = mosquitto_connect(mosq, host, port, 60);
		
	if(connectionResult == MOSQ_ERR_SUCCESS) {
			
	} else if (connectionResult == MOSQ_ERR_INVAL) {
		dataArrived = true;
		mqttConnectCallback(true, "Mosquitto MQTT ERROR: Could not connect. MOSQ_ERR_INVAL\n");
	} else if (connectionResult == MOSQ_ERR_ERRNO) {
		dataArrived = true;
		mqttConnectCallback(true, mosquitto_strerror(connectionResult));
	} else {
		dataArrived = true;
		mqttConnectCallback(true, "Mosquitto MQTT: Unknown Error\n");
	}
}

void setMosquittoCallbacks() {
	mosquitto_connect_callback_set(mosq, connectCallback);
	mosquitto_message_callback_set(mosq, messageArrivedCallback);
	mosquitto_subscribe_callback_set(mosq, subscribeCallback);
	mosquitto_publish_callback_set(mosq, publishCallback);
	mosquitto_unsubscribe_callback_set(mosq, unsubscribeCallback);
	mosquitto_disconnect_callback_set(mosq, disconnectCallback);
}

char *getMQTTHost(char *messagingurl) {
    char *host;
    char *hostp, *foop;

    host = (char *) malloc(128);

    hostp = host;
    for (foop = messagingurl; *foop != ':'; foop ++) {
        *hostp++ = *foop;
    }
    *hostp = '\0';
	return host;
}

char *getMQTTPort(char *messagingurl) {
	char *port = malloc(1 + strlen(messagingurl));
	char *temp = rindex(messagingurl, ':') + 1;
	strcpy(port, temp);
	return port;
}

void connectToMQTT(char *clientID, int qualityOfService, void (*mqttConnectCallback)(bool error, char *message)) {
	if (getUserToken() == NULL) {
		mqttConnectCallback(true, "Could not connect to the MQTT broker. Auth token is NULL. Please initialize the ClearBlade platform first\n");
	} else {
		if (qualityOfService < 0 || qualityOfService > 2)
			mqttConnectCallback(true, "Quality of service cannot be less than 0 or greater than 2\n");
		else
			qos = qualityOfService;
		
		messagingCallback = mqttConnectCallback;
		const char *id = clientID;
		const char *username = getUserToken();
		const char *password = getSystemKey();
		char *messagingurl = getMessagingURL();
		bool clean_session = true;

		char *port = getMQTTPort(messagingurl);
		int mqttPort = atoi(port);
		
		char *host = getMQTTHost(messagingurl);

		free(port);

		mosquitto_lib_init();

		mosq = mosquitto_new(id, clean_session, NULL);
		if (!mosq) {
			fprintf(stderr, "Mosquitto MQTT: Out of memory\n");
		}
		
		mosquitto_username_pw_set(mosq, username, password);

		setMosquittoCallbacks();
		
		connect(host, mqttPort, mqttConnectCallback);

		free(host);

		runloop();
	}
}

void subscribeToTopic(char *topic, void (*messageReceivedCallback)(char *receivedMessage)) {
	if (isConnected) {
		mosquitto_subscribe(mosq, NULL, topic, qos);
		runloop();
	} else {
		printf("Mosquitto MQTT: Could not subscribe. Client is not connected\n");
	}
}

void publishMessage(char *topic, char *message) {
	if (isConnected) {
		mosquitto_publish(mosq, NULL, topic, strlen(message), message, qos, true);
		runloop();
	} else {
		printf("Mosquitto MQTT: Could not publish. Client is not connected\n");
	}
}

void unsubscribeFromTopic(char *topic) {
	if (isConnected) {
		mosquitto_unsubscribe(mosq, NULL, topic);
		runloop();
	} else {
		printf("Mosquitto MQTT: Could not unsubscribe. Client is not connected\n");
	}
}

void disconnectMQTT() {
	if (isConnected) {
		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
	} else {
		printf("Client already disconnected\n");
	}
}