#ifndef _messaging_h
#define _messaging_h

#define MSG_ERR_SUCCESS 0
#define MSG_ERR_NOT_CONNECTED 1
// TODO -- enumerate error messages here

typedef void(*messageReceivedCallback_t)(char *topic, char *message);

typedef struct messagingData {
	int qos;
        struct mosquitto *mosq;
        pthread_mutex_t *lock;
	int errno;
	messageReceivedCallback_t cb;
} MessagingData;





MessagingData *connectToMQTT(char *clientID, int qos, messageReceivedCallback_t mrcb);

int subscribeToTopic(MessagingData *md, char *topic);

int publishMessage(MessagingData *md, char *topic, char *message);

int unsubscribeFromTopic(MessagingData *md, char *topic);

int disconnectMQTT(MessagingData *md);

#endif
