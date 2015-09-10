#ifndef _messaging_h
#define _messaging_h

void connectCallback(struct mosquitto *mosq, void *userdata, int result);
void messageArrivedCallback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
void publishCallback(struct mosquitto *mosq, void *userdata, int result);
void disconnectCallback(struct mosquitto *mosq, void *userdata, int result);

bool mosquittoloop();
void setDataArrivedFlag();
void runloop();
void connect(char *host, int port, void mqttConnectCallback(bool error, char *result));
void setMosquittoCallbacks();
char *getMQTTPort(char *messagingurl);
void connectToMQTT(char *clientID, int qos, void (*mqttConnectCallback)(bool error, char *message));
void subscribeToTopic(char *topic, void (*messageReceivedCallback)(char *receivedMessage));
void publishMessage(char *topic, char *message);
void unsubscribeFromTopic(char *topic);
void disconnectMQTT();

#endif