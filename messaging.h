#ifndef _messaging_h
#define _messaging_h

void connectToMQTT(char *clientID, int qos, void (*mqttConnectCallback)(bool error, char *message));
void disconnectMQTT();

#endif