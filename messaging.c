#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mosquitto.h>
#include "util.h"
#include "messaging.h"

static int qos = 0;
static bool isConnected = false;
static struct mosquitto *mosq = NULL;

void disconnectCallback(struct mosquitto *mosq, void *userdata, int result) {
	printf("Mosquitto MQTT: Client Disconnected\n");
}

void connectCallback(struct mosquitto *mosq, void *userdata, int result) {
	printf("Yes\n");
}

void logCallback(struct mosquitto *mosq, void *obj, int level, const char *str) {
	printf("Mosquitto MQTT Log: %s\n", str);
}

void connectToMQTT(char *clientID, int qualityOfService, void (*mqttConnectCallback)(bool error, char *message)) {
	if (getUserToken() == NULL) {
		mqttConnectCallback(true, "Could not connect to the MQTT broker. Auth token is NULL. Please initialize the ClearBlade platform first");
	} else {
		if (qualityOfService < 0 || qualityOfService > 2)
			mqttConnectCallback(true, "Quality of service should be between 0 and 2 inclusive");
		else
			qos = qualityOfService;
		const char *id = clientID;
		const char *username = getUserToken();
		const char *password = getSystemKey();
		char *host = getMessagingURL();
		bool clean_session = true;

		mosquitto_lib_init();

		mosq = mosquitto_new(id, clean_session, NULL);
		if (!mosq) {
			fprintf(stderr, "Mosquitto MQTT: Out of memory\n");
		}
		
		mosquitto_username_pw_set(mosq, username, password);
		mosquitto_connect_callback_set(mosq, connectCallback);
		mosquitto_disconnect_callback_set(mosq, disconnectCallback);
		mosquitto_log_callback_set(mosq, logCallback);
		
		int connectionResult = mosquitto_connect(mosq, "rtp.clearblade.com", 1883, 60);
		if(connectionResult == MOSQ_ERR_SUCCESS){
			isConnected = true;
			mqttConnectCallback(false, "Client Connected!\n");
		} else if (connectionResult == MOSQ_ERR_INVAL) {
			mqttConnectCallback(true, "Mosquitto MQTT ERROR: Could not connect. MOSQ_ERR_INVAL\n");
		} else if (connectionResult == MOSQ_ERR_ERRNO) {
			mqttConnectCallback(true, mosquitto_strerror(connectionResult));
		} else {
			mqttConnectCallback(true, mosquitto_strerror(connectionResult));
		}
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