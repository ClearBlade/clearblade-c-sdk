#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <mosquitto.h>
#include <pthread.h>
#include "util.h"
#include "messaging.h"

static int connectionCount = 0;

//
//  Forward declaration bullshit. Welcome to 1973. I was there. Wasn't
//  as cool as you think.
//
//  First, all of the callbacks needed:
//
void connectCallback(struct mosquitto *mosq, void *userdata, int result);
void messageReceivedCallback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *);
void subscribeCallback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
void publishCallback(struct mosquitto *mosq, void *userdata, int result);
void unsubscribeCallback(struct mosquitto *mosq, void *userdata, int result);
void disconnectCallback(struct mosquitto *mosq, void *userdata, int result);

void initialize_messaging() {
	mosquitto_lib_init();
}

void setMosquittoCallbacks(MessagingData *md) {
	mosquitto_connect_callback_set(md->mosq, connectCallback);
	mosquitto_message_callback_set(md->mosq, messageReceivedCallback);
	mosquitto_subscribe_callback_set(md->mosq, subscribeCallback);
	mosquitto_publish_callback_set(md->mosq, publishCallback);
	mosquitto_unsubscribe_callback_set(md->mosq, unsubscribeCallback);
	mosquitto_disconnect_callback_set(md->mosq, disconnectCallback);
}

MessagingData *initializeMessagingData(char *clientId, int qos) {
	MessagingData *md = (MessagingData *) malloc(sizeof(MessagingData));
	if (md == NULL) {
		fprintf(stderr, "MessagingData: Out of memory.");
		exit(1);
	}
	md->mosq = mosquitto_new(clientId, true, NULL);
	if (!md->mosq) {
		fprintf(stderr, "Mosquitto MQTT: Out of memory\n");
		exit(1);
	}
	md->qos = qos;
	md->errno = 0;
	setMosquittoCallbacks(md);
	pthread_mutex_lock(md->lock);
	return md;
}

void finalizeMessagingData(MessagingData *md) {
	mosquitto_disconnect(md->mosq);
	mosquitto_destroy(md->mosq);
	if (--connectionCount <= 0) {
		mosquitto_lib_cleanup();
	}
	pthread_mutex_unlock(md->lock);
	free(md);
}

void connectCallback(struct mosquitto *mosq, void *userdata, int result) {
	// TODO -- this assumes the connect worked every time. Duh.

	printf("CONNECT CALLBACK: %d\n", result);
	MessagingData *md = (MessagingData *)userdata;
	md->errno = result;
	mosquitto_loop_start(mosq);
	pthread_mutex_unlock(md->lock);
}

void subscribeCallback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) {
	MessagingData *md = (MessagingData *)userdata;
	md->errno = 0; // Don't know what to do here
	pthread_mutex_unlock(md->lock);
}

void messageReceivedCallback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
	MessagingData *md = (MessagingData *)userdata;
}

void publishCallback(struct mosquitto *mosq, void *userdata, int result) {
	MessagingData *md = (MessagingData *)userdata;
	md->errno = result;
	pthread_mutex_unlock(md->lock);
}

void unsubscribeCallback(struct mosquitto *mosq, void *userdata, int result) {
	MessagingData *md = (MessagingData *)userdata;
	md->errno = result;
	pthread_mutex_unlock(md->lock);
}

void disconnectCallback(struct mosquitto *mosq, void *userdata, int result) {
	MessagingData *md = (MessagingData *)userdata;
	md->errno = result;
	pthread_mutex_unlock(md->lock);
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

// Parses the messaging URL and returns the mqtt port
char *getMQTTPort(char *messagingurl) {
	char *port = malloc(1 + strlen(messagingurl));
	char *temp = rindex(messagingurl, ':') + 1;
	strcpy(port, temp);
	return port;
}

MessagingData *connectToMQTT(char *clientId, int qualityOfService, messageReceivedCallback_t mrcb) {
	if (getUserToken() == NULL) {
		fprintf(stderr, "connectToMQTT called with unset user token\n");
		return NULL;
	}
	if (qualityOfService < 0 || qualityOfService > 2) {
		fprintf(stderr, "Quality of service cannot be less than 0 or greater than 2\n");
		return NULL;
	}
	
	const char *username = getUserToken();
	const char *password = getSystemKey();
	char *messagingurl = getMessagingURL();

	char *port = getMQTTPort(messagingurl); // Parse port from messaging url
	int mqttPort = atoi(port);
	free(port);
	char *host = getMQTTHost(messagingurl); // Parse host from messaging url


	MessagingData *md = initializeMessagingData(clientId, qualityOfService);
	md->cb = mrcb;
	
	mosquitto_username_pw_set(md->mosq, username, password); // Set username and password
	int connectionResult = mosquitto_connect(md->mosq, host, mqttPort, 60);
	
	if (connectionResult != 0) {
		fprintf(stderr, "Mosquitto connect failed: %d\n", connectionResult);
		return NULL;
	}
	
	free(host);
	return md;
}

int subscribeToTopic(MessagingData *md, char *topic) {
	mosquitto_subscribe(md->mosq, NULL, topic, md->qos);
	pthread_mutex_lock(md->lock);
}

int publishMessage(MessagingData *md, char *topic, char *message) {
	mosquitto_publish(md->mosq, NULL, topic, strlen(message), message, md->qos, true);
	pthread_mutex_lock(md->lock);
	return md->errno;
}

int unsubscribeFromTopic(MessagingData *md, char *topic) {
	md->errno = 0;
	mosquitto_unsubscribe(md->mosq, NULL, topic);
	pthread_mutex_lock(md->lock);
	return md->errno;
}

int disconnectMQTT(MessagingData *md) {
	finalizeMessagingData(md);
	return 0;
}
