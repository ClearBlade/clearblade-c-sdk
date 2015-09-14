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
bool dataArrived = false; // This flag is to check whether data has arrived on the socket using the mosquitto loop
struct mosquitto *mosq = NULL;
void (*messagingCallback)(bool error, char *result) = NULL; // User's connect callback that we will set later in the mosquitto internal connect callback
void (*dataArrivedCallback)(char *topic, char *message) = NULL; // User's message arrived callback that we will set later in the mosquitto internal message arrived callback

/**
  * Mosquitto loop to check if data is arrived on the socket
*/
bool mosquittoloop() {
	mosquitto_loop(mosq, -1, 1);
	return dataArrived;
}

// Reinitializes the dataArrived flag
void setDataArrivedFlag() {
	dataArrived = false;
}

/**
  * This is the main loop that will run after every connect, publish, subscribe and unsubscribe to wait for the
  * corresponding action to be completed successfully
*/
void runloop() {
	while (1) {
		bool flag = mosquittoloop();
		if (flag)
			break;
	}
	sleep(1);
	setDataArrivedFlag();
}

/**
  * This loop should be used when the client has nothing to do but just wait for messages to be received on a specific topic.
  * This is highly blocking
*/
void runloopForever() {
	mosquitto_loop_forever(mosq, -1, 1);
}

/**
  * Mosquitto's internal connect callback
*/
void connectCallback(struct mosquitto *mosq, void *userdata, int result) {
	isConnected = true;
	dataArrived = true; // Connect complete
	messagingCallback(false, "Mosquitto MQTT: Client Connected"); // User provided connect callback
}

/**
  * Mosquitto's internal subscribe callback
*/
void subscribeCallback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) {
	dataArrived = true; // Subscribe complete
}

/**
  * Mosquitto's internal message arrived callback
*/
void messageArrivedCallback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
	dataArrived = true; // Message arrived
	dataArrivedCallback(message->topic, message->payload); // User provided message arrived callback
	
}

/**
  * Mosquitto's internal publish callback
*/
void publishCallback(struct mosquitto *mosq, void *userdata, int result) {
	dataArrived = true; // Publish complete
}

/**
  * Mosquitto's internal unsubscribe callback
*/
void unsubscribeCallback(struct mosquitto *mosq, void *userdata, int result) {
	dataArrived = true; // Unsubscribe complete
}

/**
  * Mosquitto's internal disconnect callback
*/
void disconnectCallback(struct mosquitto *mosq, void *userdata, int result) {
	printf("Mosquitto MQTT: Client Disconnected\n");
}

/**
  * Mosquitto's internal Log callback. Only for debugging. Currently not used
*/
void logCallback(struct mosquitto *mosq, void *obj, int level, const char *str) {
	printf("Mosquitto MQTT Log: %s\n", str);
}

/**
  * Attempts to connects to the MQTT Broker and returns error codes if any
*/
void connect(char *host, int port, void mqttConnectCallback(bool error, char *result)) {
	int connectionResult = mosquitto_connect(mosq, host, port, 60);
		
	if(connectionResult == MOSQ_ERR_SUCCESS) {
			
	} else if (connectionResult == MOSQ_ERR_INVAL) {
		dataArrived = true; // This is to let the mosquitto loop know that the client could not connect and it should stop the loop and give control back to the program
		mqttConnectCallback(true, "Mosquitto MQTT ERROR: Could not connect. MOSQ_ERR_INVAL\n");
	} else if (connectionResult == MOSQ_ERR_ERRNO) {
		dataArrived = true;
		mqttConnectCallback(true, mosquitto_strerror(connectionResult));
	} else {
		dataArrived = true;
		mqttConnectCallback(true, "Mosquitto MQTT: Unknown Error\n");
	}
}

/**
  * This sets all the required internal callbacks for mosquitto
*/
void setMosquittoCallbacks() {
	mosquitto_connect_callback_set(mosq, connectCallback);
	mosquitto_message_callback_set(mosq, messageArrivedCallback);
	mosquitto_subscribe_callback_set(mosq, subscribeCallback);
	mosquitto_publish_callback_set(mosq, publishCallback);
	mosquitto_unsubscribe_callback_set(mosq, unsubscribeCallback);
	mosquitto_disconnect_callback_set(mosq, disconnectCallback);
}

// Parses the messaging URL and returns the mqtt host
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

// Parses the messaging URL and returns the mqtt port
char *getMQTTPort(char *messagingurl) {
	char *port = malloc(1 + strlen(messagingurl));
	char *temp = rindex(messagingurl, ':') + 1;
	strcpy(port, temp);
	return port;
}

/**
  * Call this function to connect to the MQTT Broker. clientID, QoS and connect callback are the required parameters.
*/
void connectToMQTT(char *clientID, int qualityOfService, void (*mqttConnectCallback)(bool error, char *message)) {
	if (getUserToken() == NULL) {
		mqttConnectCallback(true, "Could not connect to the MQTT broker. Auth token is NULL. Please initialize the ClearBlade platform first\n");
	} else {
		if (qualityOfService < 0 || qualityOfService > 2)
			mqttConnectCallback(true, "Quality of service cannot be less than 0 or greater than 2\n");
		else
			qos = qualityOfService;
		
		messagingCallback = mqttConnectCallback; // Set the user's connect callback
		const char *id = clientID;
		const char *username = getUserToken();
		const char *password = getSystemKey();
		char *messagingurl = getMessagingURL();
		bool clean_session = true;

		char *port = getMQTTPort(messagingurl); // Parse port from messaging url
		int mqttPort = atoi(port);
		
		char *host = getMQTTHost(messagingurl); // Parse host from messaging url

		free(port);

		mosquitto_lib_init(); // Initialize mosquitto

		mosq = mosquitto_new(id, clean_session, NULL);
		if (!mosq) {
			fprintf(stderr, "Mosquitto MQTT: Out of memory\n");
		}
		
		mosquitto_username_pw_set(mosq, username, password); // Set username and password

		setMosquittoCallbacks();
		
		connect(host, mqttPort, mqttConnectCallback);

		free(host);

		runloop(); // Wait for the connection to complete. When complete go to the internal connect callback
	}
}

/**
  * Subscribes to a topic and sets the user provided message received callback
*/
void subscribeToTopic(char *topic, void (*messageReceivedCallback)(char *topic, char *receivedMessage)) {
	if (isConnected) {
		mosquitto_subscribe(mosq, NULL, topic, qos);
		runloop(); // Wait for the subscribe to complete. When complete go to the internal subscribe callback
		dataArrivedCallback = messageReceivedCallback; // Sets the user provided message arrived callback
	} else {
		printf("Mosquitto MQTT: Could not subscribe. Client is not connected\n");
	}
}

/**
  * Publishes a message to a topic
*/
void publishMessage(char *topic, char *message) {
	if (isConnected) {
		mosquitto_publish(mosq, NULL, topic, strlen(message), message, qos, true);
		runloop(); // Wait for the publish to complete. When complete go to the internal publish callback
	} else {
		printf("Mosquitto MQTT: Could not publish. Client is not connected\n");
	}
}

/**
  * Unsubscribes from a topic
*/
void unsubscribeFromTopic(char *topic) {
	if (isConnected) {
		mosquitto_unsubscribe(mosq, NULL, topic);
		runloop(); // Wait for the unsubscribe to complete. When complete go to the internal unsubscribe callback
	} else {
		printf("Mosquitto MQTT: Could not unsubscribe. Client is not connected\n");
	}
}

/**
  * Disconnects from the MQTT Broker. This also cleans up the mosquitto library and destroy's the mosquitto client
*/
void disconnectMQTT() {
	if (isConnected) {
		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
	} else {
		printf("Client already disconnected\n");
	}
}