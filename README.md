# ClearBlade C SDK

# QuickStart

## Prerequisites

To install the C SDK, you will need the following dependencies:

- libcurl
- OpenSSL
- Paho MQTT library
- Jansson JSON library

> [!IMPORTANT]
> The Paho MQTT library contains a Makefile that only supports Linux systems. You can try to install it on other operating systems, but it might not work. So, you may need to edit the Makefile for your operating system or install the C SDK on a Linux system.

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
- You will need to know where brew installed OpenSSL. The path to OpenSSL will be in `/opt/homebrew/Cellar/` and will resemble `/opt/homebrew/Cellar/openssl@3/3.1.2`

```bash
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c/
rm -r build/*
mkdir build
cd build

# Replace {YOUR_OPEN_SSL_PATH} below with the path to your OpenSSL installation (for example: /opt/homebrew/Cellar/openssl@3/3.1.2)
cmake -DPAHO_WITH_SSL=TRUE -DPAHO_HIGH_PERFORMANCE=TRUE -DOPENSSL_ROOT_DIR="{YOUR_OPEN_SSL_PATH}" ..
make
sudo make install
```

### Install the json-c Library
You can follow the instructions given @ http://json-c.github.io/json-c/json-c-current-release/doc/html/index.html to install the json-c library. In addition, you may be able to install the json-c library through a package manager.

```
sudo apt install libjson-c-dev
```

### Install the C SDK

Execute the following commands to install the C SDK on your system:

```bash
git clone https://github.com/ClearBlade/ClearBlade-C-SDK.git
cd ClearBlade-C-SDK/
make
sudo make install
```

# API reference

## Logging
The ClearBlade C SDK utilizes the log.c library https://github.com/rxi/log.c to enable log levels.

The log levels supported are:
  * LOG_TRACE
  * LOG_DEBUG
  * LOG_INFO
  * LOG_WARN
  * LOG_ERROR
  * LOG_FATAL

### Enabling log levels
By default, the SDK will log at the __LOG_TRACE__ level. To enable a different level of logging, invoke the __log_set_level(int level)__ function in your code and supply one of the log levels listed above.

### Turning off logging
Logging can be disabled and turned off completely by invoking the __log_set_quiet(bool enable)__ function and passing __true__ as the parameter.

## Initialize and authenticate

> [!IMPORTANT]
> You must initialize and authenticate with the ClearBlade Platform before you perform any other functions.

If you have not installed the prerequisites, please follow the tutorial @ [Prerequisites](../Quickstart#prerequisites). After you have installed all the prerequisites and the C SDK, include the **_clearblade.h_** header in your code: `#include <clearblade.h>`  

### User Authentication

```C
void cbInitCallback(void *context, bool error, char *result) {
  if(error) {
    printf("ClearBlade init failed %s\n", result);
    exit(-1);
  } else {
    printf("ClearBlade Init Succeeded\nAuth token: %s\n", result);
  }
}

//void cbInitialize(void *context, char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword, void (*initCallback)(void *context, bool error, char *result))

cbInitialize(NULL, SYSTEM_KEY, SYSTEM_SECRET, PLATFORM_URL, MESSAGING_URL, USER_EMAIL, USER_PASSWORD, &cbInitCallback);
```

You will need to pass the function a context, your **SYSTEM_KEY**, **SYSTEM_SECRET**, **PLATFORM_URL** (https://platform.clearblade.com or similar), **MESSAGING_URL**, **USER_EMAIL**, **USER_PASSWORD**, and a function as a callback. After successful authentication, you will receive an authentication token in the callback. You can choose to store it in a variable, but the SDK stores a copy of it for itself.

**MESSAGING_URL** can be:  
tcp://platform.clearblade.com:1883, or similar, for unsecured messaging.  
ssl://platform.clearblade.com:1884, or similar, for secured messaging. i.e. over TLS

> [!NOTE]
> The first argument passed to the cbInitialize function is a ```(void*) context``` argument that provides the ability to receive application specific contextual information in the init callback function.

### Device Authentication
```C
void cbInitCallback(void *context, bool error, char *result) {
  if(error) {
    printf("ClearBlade init failed %s\n", result);
    exit(-1);
  } else {
    printf("ClearBlade Init Succeeded\nAuth token: %s\n", result);
  }
}

//void initializeCBAsDevice(void *context, char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *devicename, char *activekey, void (*initCallback)(void *context, bool error, char *result))
initializeCBAsDevice(NULL, SYSTEM_KEY, SYSTEM_SECRET, PLATFORM_URL, MESSAGING_URL, DEVICE_NAME, ACTIVE_KEY, &cbInitCallback);
```

You will need to pass the function a context, your **SYSTEM_KEY**, **SYSTEM_SECRET**, **PLATFORM_URL** (https://platform.clearblade.com or similar), **MESSAGING_URL**, **DEVICE_NAME**, **ACTIVE_KEY**, and a function as a callback. After successful authentication, you will receive an authentication token in the callback. You can choose to store it in a variable, but the SDK stores a copy of it for itself.

**MESSAGING_URL** can be:  
tcp://platform.clearblade.com:1883, or similar, for unsecured messaging.  
ssl://platform.clearblade.com:1884, or similar, for secured messaging. i.e. over TLS

> [!NOTE]
> The first argument passed to the initializeCBAsDevice function is a ```(void*) context``` argument that provides the ability to receive application specific contextual information in the init callback function.

### Device Authentication With mTLS
Rather than authenticating a device with the device name and active key, a device can be authenticated using mTLS. This requires the device to pass the location of the x509 SSL certificate and the location of the private key. The ClearBlade Platform will then ensure the certificate and private key passed by the device is valid and matches what the ClearBlade Platform expects.

If the device does not exist in the system it will be automatically created, providing the ability to do Just-In_Time provisioning.

> [!WARNING]
> Using mTLS authentication requires additional configuration by ClearBlade that is not implemented by default. If you require mTLS authentication, you will need to contact ClearBlade.


```C
void cbInitCallback(void *context, bool error, char *result) {
  if(error) {
    printf("ClearBlade init failed %s\n", result);
    exit(-1);
  } else {
    printf("ClearBlade Init Succeeded\nAuth token: %s\n", result);
  }
}

initializeClearBladeAsMtlsDevice(NULL, SYSTEM_KEY, SYSTEM_SECRET, PLATFORM_URL, MESSAGING_URL, DEVICE_NAME, CERT_FILE, KEY_FILE, &cbInitCallback);
```

You will need to pass the function an optional **context**, your **SYSTEM_KEY**, **SYSTEM_SECRET**, **PLATFORM_URL** (https://platform.clearblade.com or similar), **MESSAGING_URL**, **DEVICE_NAME**, **CERT_FILE**, **KEY_FILE**, and a function as a callback. After successful authentication, you will receive an authentication token in the callback. You can choose to store it in a variable, but the SDK stores a copy of it for itself.

CERT_FILE can be either the path to the SSL certificate or the contents of the certificate file
KEY_FILE can be either the path to the private key or the contents of the private key file

**MESSAGING_URL** can be:  
tcp://platform.clearblade.com:1883, or similar, for unsecured messaging.  
ssl://platform.clearblade.com:1884, or similar, for secured messaging. i.e., over TLS

> [!NOTE]
> The first argument passed to the initializeCBAsDevice function is a ```(void*) context``` argument that provides the ability to receive application specific contextual information in the init callback function.


# MQTT functions

## Connect to the MQTT broker

**Before you connect to the MQTT broker, include the `#include "MQTTAsync.h"` header in your code and link the `-lpaho-mqtt3as` library when you compile your code**.

```C
void connectCbMQTT(void* context, char *clientId, CbMqttConnectOptions *options,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback)
```

### Connection Options
The CbMqttConnectOptions struct provides a way to specify advanced connection options when connecting to the MQTT broker. The C SDK will default the connection options to appropriate values if the options object is not provided. In addition, the getDefaultCbMQTTConnectOptions() function provides a convenient way to create the connection options with appropriate default values. 

> [!NOTE]
> If the getDefaultCbMQTTConnectOptions function is used, it is the responsibility of the calling application to __free__ the memory used by the connection options.

typedef struct {
  int keepAliveInterval;
  bool cleanSession;
  int maxInFlight;
  int connectTimeout;
  int retryInterval;
	bool automaticReconnect;
	int minRetryInterval;
	int maxRetryInterval;
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;
	MQTTAsync_willOptions* will;
} CbMqttConnectOptions;

Property Name | Default Value | Description
------------- | ------------- | -----------
keepAliveInterval | 60 | Measured in seconds, defines the maximum time interval between messages sent or received
cleanSession | true | Indicates whether the client and server should remember state for the client across reconnects
maxInFlight | 65535 | Limits how many messages can be sent without receiving acknowledgments
connectTimeout | 30 | Measured in seconds, defines the maximum time interval the client will wait for the network connection to the MQTT server to be established
retryInterval | 0 | The time interval in seconds after which unacknowledged publish requests are retried during a TCP session
automaticReconnect | false | Will the client library attempt to reconnect automatically in the event of a connection failure
minRetryInterval | 1 | The minimum times before the next connection attempt. At each failure to reconnect, the retry interval is doubled until the maximum value is reached, and there it stays until the connection is successfully re-established whereupon it is reset.
maxRetryInterval | 60 | The  maximum times before the next connection attempt. At each failure to reconnect, the retry interval is doubled until the maximum value is reached, and there it stays until the connection is successfully re-established whereupon it is reset.
onSuccess | NULL | A callback function that will be invoked when the connection to the message broker is successful
onFailure | NULL |A callback function that will be invoked when the connection to the message broker fails
will | NULL | This is a pointer to a MQTTClient_willOptions structure.

```C
char *clientID = "test-client";
CbMqttConnectOptions* connectOptions;

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("MQTT Connect failed, rc %d\n", response ? response->code : 0);
  printf("Context received: %s\n", (char const *)context);

  free(connectOptions);
}

void onConnectSuccess(void* context, MQTTAsync_successData* response) {
  printf("Successful connection to MQTT Broker\n");

  free(connectOptions);
}

void onConnectionLost(void *context, char *cause) {
    printf("\nConnection lost\n");
  	printf("Cause: %s\n", cause);
    free(connectOptions);
}

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

connectOptions = getDefaultCbMQTTConnectOptions();

connectOptions->keepAliveInterval = 30;
connectOptions->cleanSession = false;
connectOptions->minRetryInterval = 10;
connectOptions->maxRetryInterval = 30;
connectOptions->onSuccess = onConnectSuccess;
connectOptions->onFailure = onConnectFailure;

connectCbMQTT(NULL, clientID, connectOptions, messageArrived, onConnectionLost);

``` 

## Response Options
The CbMqttResponseOptions struct provides a way to specify advanced options when publishing, subscribing, and unsubscribing. The C SDK will default the response options to appropriate values if the options object is not provided. In addition, the getDefaultCbMQTTResponseOptions() function provides a convenient way to create the response options with appropriate default values. 

> [!NOTE]
> If the getDefaultCbMQTTResponseOptions function is used, it is the responsibility of the calling application to __free__ the memory used by the response options.

typedef struct  {
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;
	CbMqttSubscribeOptions* subscribeOptions;
} CbMqttResponseOptions;

Property Name | Default Value | Description
------------- | ------------- | -----------
onSuccess | NULL | A callback function that will be invoked when the publish, subscribe, or unsubscribe is successful
onFailure | NULL |A callback function that will be invoked when the publish, subscribe, or unsubscribe fails
subscribeOptions | NULL | This is a pointer to a CbMqttSubscribeOptions structure.

## Subscribe Options
The CbMqttSubscribeOptions struct provides a way to specify advanced options when subscribing to topics. The C SDK will default the options to appropriate values if the options object is not provided. In addition, the getDefaultCbMQTTSubscribeOptions() function provides a convenient way to create the subscribe options with appropriate default values. 

> [!NOTE]
> If the getDefaultCbMQTTSubscribeOptions function is used, it is the responsibility of the calling application to __free__ the memory used by the subscribe options.

typedef struct  {
  int noLocal;
	int retainAsPublished;
	int retainHandling;
} CbMqttSubscribeOptions;

Property Name | Default Value | Description
------------- | ------------- | -----------
noLocal | 0 | To not receive our own publications, set to 1. 0 is the original MQTT behaviour - all messages matching the subscription are received.
retainAsPublished | 0 | To keep the retain flag as on the original publish message, set to 1. If 0, defaults to the original MQTT behaviour where the retain flag is only set on publications sent by a broker if in response to a subscribe request.
retainHandling | 0 | 0 - send retained messages at the time of the subscribe (original MQTT behaviour), 1 - send retained messages on subscribe only if the subscription is new, 2 - do not send retained messages at all

## Publish a message

To publish a message once a successful connection has been established to the MQTT broker, use the `publishCbMQTT()` function:

```C
CbMqttResponseOptions* publishOptions;
char *message = "Hello, World!!";
char *topic = "AwesomeTopic";
int qos = 0;
int retained = 0;

void onPublishFailure(void* context, MQTTAsync_failureData* response) {
	printf("Publish failed, rc %d\n", response ? response->code : 0);
  printf("Context received: %s\n", (const char*)context);

  free(publishOptions);
}

void onPublishSuccess(void* context, MQTTAsync_successData* response) {
  printf("Publish successful\n");
  printf("Context received: %s\n", (const char*)context);

  free(publishOptions);
}

//void publishCbMQTT(void* context, char *message, char *topic, int qos, int retained, CbMqttResponseOptions* options)
publishOptions = getDefaultCbMQTTResponseOptions();
publishOptions->onSuccess = onPublishSuccess;
publishOptions->onFailure = onPublishFailure;

publishCbMQTT(NULL, message, topic, qos , retained, options);
```

## Subscribe to a topic

To subscribe to a topic, use the `subscribeToTopic()` function:

```C
CbMqttResponseOptions* responseOptions;
CbMqttSubscribeOptions* subscribeOptions;

char *topic = "AwesomeTopic";
int qos = 0;

void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("subscribe failed, rc %d\n", response ? response->code : 0);

  free(responseOptions);
  free(subscribeOptions);
}

void onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
  printf("Subscribe successful\n");

  free(responseOptions);
  free(subscribeOptions);
}

responseOptions = getDefaultCbMQTTResponseOptions();
subscribeOptions getDefaultCbMQTTSubscribeOptions();

responseOptions->onSuccess = onSubscribeSuccess;
responseOptions->onFailure = onSubscribeFailure;
responseOptions->subscribeOptions = subscribeOptions;

//void subscribeCbMQTT(void* context, char *topic, int qos, CbMqttResponseOptions* options)
subscribeCbMQTT(NULL, topicName, qos, responseOptions);
```

## Unsubscribe from a topic

To unsubscribe from a topic, use the `unsubscribeCbMQTT()` function:

```C
CbMqttResponseOptions* unsubscribeOptions;
char *topic = "AwesomeTopic";


unsubscribeOptions = getDefaultCbMQTTResponseOptions();
unsubscribeOptions->onSuccess = onUnsubscribeSuccess;
unsubscribeOptions->onFailure = onUnsubscribeFailure;

//void unsubscribeCbMQTT(void* context, char *topic, CbMqttResponseOptions* options)
unsubscribeCbMQTT((void*)"Context from topicUnsubscribe", topicName, unsubscribeOptions);
```

## Disconnect from the MQTT broker

To disconnect your client from the MQTT broker, call the `disconnectMQTTClient()` function.

### Disconnect Options
The CbMqttDisconnectOptions struct provides a way to specify advanced disconnect options when disconnecting from the MQTT broker. The C SDK will default the disconnect options to appropriate values if the options object is not provided. In addition, the getDefaultCbMQTTDisconnectOptions() function provides a convenient way to create the disconnection options with appropriate default values. 

> [!NOTE]
> If the getDefaultCbMQTTDisconnectOptions function is used, it is the responsibility of the calling application to __free__ the memory used by the disconnect options.

typedef struct {
  int timeout;
	MQTTAsync_onSuccess* onSuccess;
	MQTTAsync_onFailure* onFailure;
} CbMqttDisconnectOptions;

Property Name | Default Value | Description
------------- | ------------- | -----------
timeout | 0 | The client delays disconnection for up to this time (in milliseconds) in order to allow in-flight message transfers to complete.
onSuccess | NULL | A callback function that will be invoked when the disconnect from the message broker is successful
onFailure | NULL |A callback function that will be invoked when the disconnect from the message broker fails

```C
CbMqttDisconnectOptions* disconnectOptions;

void onDisonnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("disconnect failed, rc %d\n", response ? response->code : 0);

  free(disconnectOptions);
}

void onDisconnectSuccess(void* context, MQTTAsync_successData* response) {
  printf("Successful disconnection from MQTT Broker\n");

  free(disconnectOptions);
}

disconnectOptions = getDefaultCbMQTTDisconnectOptions();
disconnectOptions->timeout = 20;
disconnectOptions->onSuccess = onDisconnectSuccess;
disconnectOptions->onFailure = onDisonnectFailure;

//void disconnectCbMQTT(void* context, CbMqttDisconnectOptions *options)
disconnectCbMQTT(NULL, disconnectOptions);

```

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
