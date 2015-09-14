#ifndef _messaging_h
#define _messaging_h



/**
  * This loop should be used when the client has nothing to do but just wait for messages to be received on a specific topic.
  * This is highly blocking
*/
void runloopForever();


/**
  * This is the main loop that will run after every connect, publish, subscribe and unsubscribe to wait for the
  * corresponding action to be completed successfully
*/
void runloop();


/**
  * Call this function to connect to the MQTT Broker. clientID, QoS and connect callback are the required parameters.
*/
void connectToMQTT(char *clientID, int qos, void (*mqttConnectCallback)(bool error, char *message));


/**
  * Subscribes to a topic and sets the user provided message received callback
*/
void subscribeToTopic(char *topic, void (*messageReceivedCallback)(char *topic, char *receivedMessage));


/**
  * Publishes a message to a topic
*/
void publishMessage(char *topic, char *message);


/**
  * Unsubscribes from a topic
*/
void unsubscribeFromTopic(char *topic);


/**
  * Disconnects from the MQTT Broker. This also cleans up the mosquitto library and destroys the mosquitto client
*/
void disconnectMQTT();

#endif