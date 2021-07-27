# ClearBlade C SDK

# API reference

## Initialize and authenticate

```markdown
> **Heads Up!**
You must initialize and authenticate with the ClearBlade Platform before you perform any other functions.
```

If you still need to install the prerequisites, please follow the tutorial @ [Prerequisites](../Quickstart#prerequisites). After you have installed all the prerequisites and the C SDK, include the **_clearblade.h_** header in your code: `#include <clearblade.h>`  
Call the `initializeClearBlade()` or `initializeClearBladeDevice()` function to initialize and authenticate your user with ClearBlade:

```C
void cbInitCallback(bool error, char *result) {
  if(error) {
    printf("ClearBlade init failed %s\n", result);
    exit(-1);
  } else {
    printf("ClearBlade Init Succeeded\nAuth token: %s\n", result);
  }
}

initializeClearBlade(SYSTEM_KEY, SYSTEM_SECRET, PLATFORM_URL, MESSAGING_URL, USER_EMAIL, USER_PASSWORD, &cbInitCallback);
```

You must pass the function: Your **SYSTEM_KEY**, **SYSTEM_SECRET**, **PLATFORM_URL** (https://platform.clearblade.com or similar), **MESSAGING_URL**, **USER_EMAIL**,
**USER_PASSWORD** (or **DEVICE_NAME** and **ACTIVE_KEY** if authenticating as a device) and a function as a callback. After successful authentication, you will receive an authentication token in the callback. You can store it in a variable, but the SDK stores a copy of it for itself.

**MESSAGING_URL** can be:  
tcp://platform.clearblade.com:1883, or similar, for unsecured messaging.  
ssl://platform.clearblade.com:1884, or similar, for secured messaging. i.e., over TLS

# MQTT functions

## Connect to the MQTT broker

**Before you connect to the MQTT broker, include the `#include "MQTTAsync.h"` header in your code and link the `-lpaho-mqtt3as` library when you compile your code**. There are two functions to choose from when connecting to the MQTT broker with the following signatures:

```C
connectToMQTT(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message))
```

**OR**

```C
connectToMQTTAdvanced(char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, MQTTAsync_successData* response),
 									int (*messageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTAsync_message *message),
									void (*onConnLostCallback)(void *context, char *cause))
```

The difference between the two is that the first one does not take an **onConnectionLost** callback, whereas the second one does. This means if the connection to the MQTT broker is lost, and you choose the first function, the code will notify you and try to reconnect once, leaving everything to the **Gods of MQTT**. But if you use the second function, you can handle the connection lost event and make a decision accordingly. **We recommend you use the second function.**  
Here's an example of how you would connect to the MQTT broker using the `connectToMQTT()` function:

```C
char *clientID = "test-client";
int qos = 0;

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
  printf("Message arrived\n");
  printf("Topic: %s\n", topicName);
  char *messagePayload = malloc(message->payloadlen + 1);
  strncpy(messagePayload, message->payload, message->payloadlen);
  messagePayload[message->payloadlen] = '\0';
  printf("Message: %s\n", messagePayload);

  MQTTAsync_freeMessage(&message);
  MQTTAsync_free(topicName);
  free(messagePayload);

  return 1;
}


void onConnect(void* context, MQTTAsync_successData* response) {
  printf("Successful connection to MQTT Broker\n");
  extern int finished; // Get the 'finished' variable from the CB SDK and set it to 1 to stop the connect loop
  finished = 1;
}

connectToMQTT(clientID, qos, &onConnect, &messageArrived);
```

To the `connectToMQTT()` function, you need to pass the client-ID (string), quality of service (int), a callback to notify you of a successful connection, and a callback to notify you when a message arrives on a subscription.  
{{< note title="Note" >}}
In the **onConnect** callback, grab the finished variable from the C SDK using the **extern** keyword and set it to 1. This tells the SDK that the connect call was completed successfully, and the C SDK can stop its internal loop that was waiting for the connection to complete.
{{< /note >}}

Now, to use the `connectToMQTTAdvanced()` function, use the following code as a template:

```C
char *clientID = "test-client";
int qos = 0;

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
  printf("Message arrived\n");
  printf("Topic: %s\n", topicName);
  char *messagePayload = malloc(message->payloadlen + 1);
  strncpy(messagePayload, message->payload, message->payloadlen);
  messagePayload[message->payloadlen] = '\0';
  printf("Message: %s\n", messagePayload);

  MQTTAsync_freeMessage(&message);
  MQTTAsync_free(topicName);
  free(messagePayload);

  return 1;
}


void onConnect(void* context, MQTTAsync_successData* response) {
  printf("Successful connection to MQTT Broker\n");
  extern int finished; // Get the 'finished' variable from the CB SDK and set it to 1 to stop the connect loop
  finished = 1;
}

void onConnectionLost(void *context, char *cause) {
    printf("\nConnection lost\n");
  	printf("Cause: %s\n", cause);
  	printf("Reconnecting\n");
    // If you believe that your authentication token is expired, you can reauthenticate here as follows
    initializeClearBlade(SYSTEM_KEY, SYSTEM_SECRET, PLATFORM_URL, MESSAGING_URL, USER_EMAIL, USER_PASSWORD, &cbInitCallback);
    // Here cbInitCallback is the callback you defined when you first called the initializeClearBlade() function. You can change this to another callback of your choice
    connectToMQTTAdvanced(clientID, qos, &onConnect, &messageArrived, &onConnLostCallback); // This reconnects to MQTT
}

connectToMQTTAdvanced(clientID, qos, &onConnect, &messageArrived, &onConnLostCallback);
```

In this case, the extra argument is the **OnConnectionLost** callback.
{{< note title="Note" >}}
In the **onConnect** callback, grab the finished variable from the C SDK using the **extern** keyword and set it to 1. This tells the SDK that the connect call was completed successfully, and the C SDK can stop its internal loop that was waiting for the connection to complete.
{{< /note >}}

## Publish a message

To publish a message once a successful connection has been established to the MQTT broker, use the `publishMessage()` function:

```C
char *message = "Hello, World!!";
char *topic = "AwesomeTopic";
int qos = 0;
int retained = 0;
publishMessage(message, topic, qos , retained);
```

## Subscribe to a topic

To subscribe to a topic, use the `subscribeToTopic()` function:

```C
char *topic = "AwesomeTopic";
int qos = 0;
subscribeToTopic(topic, qos);
```

## Unsubscribe from a topic

To unsubscribe from a topic, use the `unsubscribeFromTopic()` function:

```C
char *topic = "AwesomeTopic";
unsubscribeFromTopic(topic);
```

## Disconnect from the MQTT broker

To disconnect your client from the MQTT broker, call the `disconnectMQTTClient()` function.

# Code service functions

You can execute a code service using the C SDK. There are two ways to do it, one with parameters you want to pass to a code service and one without.

## Execute a code service without parameters

To execute a code service without passing any parameters, use the `executeCodeServiceWithoutParams()` function:

```C
char *serviceName = "TestService";

void codeServiceCallback(bool error, char *result) {
  if(error) {
    printf("Cannot execute Code Service: %s\n", result);
  } else {
    printf("Execution successful: %s\n", result);
    free(result);
  }
}

executeCodeServiceWithoutParams(serviceName, &codeServiceCallback);
```

## Execute a code service with parameters

To execute a code service by passing parameters, use the `executeCodeServiceWithParams()` function:

```C
char *serviceName = "TestService";
char *params = "{\"name\": \"Bubba\"}";

void codeServiceCallback(bool error, char *result) {
  if(error) {
    printf("Cannot execute Code Service: %s\n", result);
  } else {
    printf("Execution successful: %s\n", result);
    free(result);
  }
}

executeCodeServiceWithParams(serviceName, params, &codeServiceCallback);
```

# QuickStart

## Prerequisites

To install the C SDK, you will need the following dependencies:

- libcurl
- OpenSSL
- Paho MQTT library
- Jansson JSON library

```markdown
> **Heads Up!**
The Paho MQTT library contains a Makefile that only supports Linux systems. You can try to install it on other operating systems, but it might not work. 
So, you may need to edit the Makefile for your operating system or install the C SDK on a Linux system.
```

### Installing **_libcurl_** and **_openssl_**

- You can download and install **_libcurl_** library from https://curl.haxx.se/libcurl/. Depending on your operating system, you may download the compiled libraries or build from the source.
- You can also download and install the **_openssl_** library from https://www.openssl.org.

### Installing the Paho MQTT library

To install the Paho MQTT library, execute the following commands:

#### Linux
```bash
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c/
make
sudo make install
```

#### Mac OS
- You will need to know where brew installed OpenSSL. The path to OpenSSL will be in `/usr/local/Cellar/` and will resemble `/usr/local/Cellar/openssl@1.1/1.1.1k`

```bash
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c/
rm -r build/*

# Replace {YOUR_OPEN_SSL_PATH} below with the path to your OpenSSL installation (for example: /usr/local/Cellar/openssl@1.1/1.1.1k)
cmake -DPAHO_WITH_SSL=TRUE -DPAHO_HIGH_PERFORMANCE=TRUE -DOPENSSL_ROOT_DIR="{YOUR_OPEN_SSL_PATH}" ../
make
sudo make install
```

### Install the Jansson JSON Library

You can follow the instructions given @ http://jansson.readthedocs.io/en/2.10/gettingstarted.html#compiling-and-installing-jansson to install the Jansson JSON library.

### Install the C SDK

Execute the following commands to install the C SDK on your system:

```bash
git clone https://github.com/ClearBlade/ClearBlade-C-SDK.git
cd ClearBlade-C-SDK/
make
sudo make install
```
