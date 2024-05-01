#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "MQTTAsync.h"
#include "util.h"
#include "messaging.h"

// TODO - Ideally, we would want separate callbacks for each client. In addition, we would want
//separate subscribe and publish callbacks for each topic.
typedef struct {
  void* context;
	MQTTAsync_onSuccess* onSuccess;
  MQTTAsync_onFailure* onFailure;
} ContextWrapper;

MQTTAsync mqttClient = NULL;
int qos = 0;

CbMqttCallbacks callbacks = {NULL, NULL, NULL};

CbMqttConnectOptions* getDefaultCbMQTTConnectOptions() {
	CbMqttConnectOptions* options = malloc(sizeof(CbMqttConnectOptions));

	//These defaults are inline with what paho uses in the MQTTAsync_connectOptions_initializer
	options->keepAliveInterval = 60;
	options->cleanSession = true;
	options->maxInFlight = 65535;
	options->connectTimeout = 30;
	options->retryInterval = 0;
	options->automaticReconnect = false;
	options->minRetryInterval = 1;
	options->maxRetryInterval = 60;
	options->onSuccess = NULL;
	options->onFailure = NULL;
	options->will = NULL;

	return options;
}

CbMqttDisconnectOptions* getDefaultCbMQTTDisconnectOptions() {
	CbMqttDisconnectOptions* options = malloc(sizeof(CbMqttDisconnectOptions));

	//These defaults are inline with what paho uses in the MQTTAsync_disconnectOptions_initializer
	options->timeout = 0;
	options->onSuccess = NULL;
	options->onFailure = NULL;

	return options;
}

CbMqttSubscribeOptions* getDefaultCbMQTTSubscribeOptions() {
	//These defaults are inline with what paho uses in the MQTTSubscribe_options_initializer
	CbMqttSubscribeOptions* options = malloc(sizeof(CbMqttSubscribeOptions));
	options->noLocal = 0;
	options->retainAsPublished = 0;
	options->retainHandling = 0;

	return options;
}

CbMqttResponseOptions* getDefaultCbMQTTResponseOptions() {
	CbMqttResponseOptions* options = malloc(sizeof(CbMqttResponseOptions));
	options->onSuccess = NULL;
	options->onFailure = NULL;
	options->subscribeOptions = NULL;

	return options;
}

int _sdk_onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
	printf("C SDK - _sdk_onMessageArrived\n");
	if (callbacks.messageArrived != NULL) {
		return callbacks.messageArrived(context, topicName, topicLen, message);
	} else {
		return 1; //Returning one indicates the message has been successfully handled: https://eclipse.github.io/paho.mqtt.c/MQTTAsync/html/_m_q_t_t_async_8h.html#a3918ead59b56816a8d7544def184e48e
	}
}

void _sdk_onConnectionSuccess(void *context, MQTTAsync_successData *response) {
	printf("C SDK - _sdk_onConnectionSuccess\n");

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onSuccess != NULL) {
			contextWrapper->onSuccess(contextWrapper->context, response);
		}
		//TODO - The context passed to the MQTTAsync_connect function appears to be passed to
		//all of the connect callbacks. We need to figure out how and when to free it.
		//free(contextWrapper);
	}
}

void _sdk_onConnectionFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - _sdk_onConnectionFailure\n");

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onFailure != NULL) {
			contextWrapper->onFailure(contextWrapper->context, response);
		}
		//TODO - The context passed to the MQTTAsync_connect function appears to be passed to
		//all of the connect callbacks. We need to figure out how and when to free it.
		//free(contextWrapper);
	}
}

void _sdk_onConnectionLost(void *context, char *cause) {
	printf("\nC SDK - _sdk_onConnectionLost: MQTT Connection lost, Cause: %s\n", cause);

	if (callbacks.onConnectionLost != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if(contextWrapper->context != NULL) callbacks.onConnectionLost(contextWrapper->context, cause);
		else callbacks.onConnectionLost(NULL, cause);

		//TODO - The context passed to the MQTTAsync_connect function appears to be passed to
		//all of the connect callbacks. We need to figure out how and when to free it.
		//free(contextWrapper);
	}
}

void _sdk_onConnectionResumed(void *context, char *cause) {
	printf("C SDK - _sdk_onConnectionResumed\n");

	if (callbacks.onConnectResumedCallback != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;

		if (contextWrapper->context != NULL) callbacks.onConnectResumedCallback(contextWrapper->context, cause);
		else callbacks.onConnectResumedCallback(NULL, cause);

		//TODO - The context passed to the MQTTAsync_connect function appears to be passed to
		//all of the connect callbacks. We need to figure out how and when to free it.
		//free(contextWrapper);
	}
}

void connectToMQTT(char *clientId, int qualityOfService, MQTTAsync_onSuccess* mqttOnConnect, MQTTAsync_messageArrived* messageArrivedCallback) {
	printf("C SDK - connectToMQTT\n");
	connectToMQTTAdvanced(clientId, qualityOfService, mqttOnConnect, messageArrivedCallback, NULL, false);
}

void connectToMQTTAdvanced(char *clientId, int qualityOfService, MQTTAsync_onSuccess* mqttOnConnect,
 		MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback, bool autoReconnect) {
	
	printf("C SDK - connectToMQTTAdvanced\n");

	char *username = getUserToken();
	const char *password = getSystemKey();
	const char *messagingurl = getMessagingURL();

	//If we don't have a user token, retrieve the device token
	if (username == NULL) {
		username = getDeviceToken();
	}

	if (username == NULL) {
		fprintf(stderr, "connectToMQTT called with unset auth token\n");
		return;
	}
	if (qualityOfService < 0 || qualityOfService > 2) {
		fprintf(stderr, "Quality of service cannot be less than 0 or greater than 2\n");
		return;
	}

	qos = qualityOfService;

	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	MQTTAsync_create(&client, messagingurl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	
	//Set callbacks
	//We have to use malloc so that the struct is allocated on the heap and not lost when the
	//function exits
	ContextWrapper* contextWrapper = (ContextWrapper*)malloc(sizeof(ContextWrapper));
	contextWrapper->context = NULL;
	contextWrapper->onSuccess = NULL;
	contextWrapper->onFailure = NULL;

	if (mqttOnConnect != NULL) {
		contextWrapper->onSuccess = mqttOnConnect;
	}

	if(onConnLostCallback != NULL) {
		callbacks.onConnectionLost = NULL;
	} else {
		callbacks.onConnectionLost = onConnLostCallback;
	}

	if(messageArrivedCallback == NULL) {
		callbacks.messageArrived = NULL;
	} else {
		callbacks.messageArrived = messageArrivedCallback;
	}
	MQTTAsync_setCallbacks(client, NULL, _sdk_onConnectionLost, _sdk_onMessageArrived, NULL);

	conn_opts.context = contextWrapper;

	if(autoReconnect) {
		conn_opts.automaticReconnect = 1;
	}
	conn_opts.keepAliveInterval = 60;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = _sdk_onConnectionSuccess;
	conn_opts.onFailure = _sdk_onConnectionFailure;
	conn_opts.username = username;
	conn_opts.password = password;

	// If protocol is 'ssl' TLS is desired
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	if (strstr(messagingurl, "ssl://") == messagingurl) {
		conn_opts.ssl = &ssl_opts;
	};

  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("Failed to start connect, return code %d\n", rc);
		MQTTAsync_destroy(&client);
    return;
  }

	mqttClient = client;
}

void connectCbMQTT(void* context, char *clientId, CbMqttConnectOptions *options,
 	MQTTAsync_messageArrived* messageArrivedCallback, MQTTAsync_connectionLost* onConnLostCallback, MQTTAsync_connected* onConnectResumedCallback) {
	MQTTAsync client;
	int rc;
	
	char *username = getUserToken();
	if (username == NULL) {
		username = getDeviceToken();
	}
	const char *password = getSystemKey();
	const char *messagingurl = getMessagingURL();

	//Use https://github.com/hivemq-cloud/paho-C-mqtt-client-example/blob/master/main.c as sample code
	if (username == NULL) {
		fprintf(stderr, "connectCbMQTT called with unset auth token\n");
		return;
	}

	//Populate context wrapper
	//We have to use malloc so that the struct is allocated on the heap and not lost when the
	//function exits
	ContextWrapper* contextWrapper = (ContextWrapper*)malloc(sizeof(ContextWrapper));
	contextWrapper->context = NULL;
	contextWrapper->onSuccess = NULL;
	contextWrapper->onFailure = NULL;

	if (context != NULL) {
		contextWrapper->context = context;
	}

	if (options->onSuccess != NULL) {
		contextWrapper->onSuccess = options->onSuccess;
	}

	if (options->onFailure != NULL) {
		contextWrapper->onFailure = options->onFailure;
	}

  if ((rc = MQTTAsync_create(&client, messagingurl, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS) {
    printf("Failed to create client, return code %d\n", rc);
  	return;
  }

	//Populate the callbacks
	if(onConnLostCallback == NULL) {
		callbacks.onConnectionLost = NULL;
	} else {
		callbacks.onConnectionLost = onConnLostCallback;
	}

	if(messageArrivedCallback == NULL) {
		callbacks.messageArrived = NULL;
	} else {
		callbacks.messageArrived = messageArrivedCallback;
	}

	if(onConnectResumedCallback == NULL) {
		callbacks.onConnectResumedCallback = NULL;
	} else {
		callbacks.onConnectResumedCallback = onConnectResumedCallback;
		MQTTAsync_setConnected(client, contextWrapper, _sdk_onConnectionResumed);
	}

  if ((rc = MQTTAsync_setCallbacks(client, contextWrapper, _sdk_onConnectionLost, _sdk_onMessageArrived, NULL)) != MQTTASYNC_SUCCESS) {
    printf("Failed to set callbacks, return code %d\n", rc);
  	MQTTAsync_destroy(&client);
		return;
	}

	//Create MQTT connection options
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	conn_opts.onSuccess = _sdk_onConnectionSuccess;
	conn_opts.onFailure = _sdk_onConnectionFailure;

	conn_opts.keepAliveInterval = options->keepAliveInterval;
	conn_opts.cleansession = options->cleanSession;
	conn_opts.maxInflight = options->maxInFlight;

	//Add the last will and testament options
	if(options->will != NULL) {
		conn_opts.will = options->will;
	}

	conn_opts.username = username;
	conn_opts.password = password;
	conn_opts.connectTimeout = options->connectTimeout;
	conn_opts.retryInterval = options->retryInterval;

	// If protocol is 'ssl' TLS is desired
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	if (strstr(messagingurl, "ssl://") == messagingurl) {
		conn_opts.ssl = &ssl_opts;
	};
	
	conn_opts.context = contextWrapper;
	conn_opts.automaticReconnect = options->automaticReconnect;
	conn_opts.minRetryInterval = options->minRetryInterval;
	conn_opts.maxRetryInterval = options->maxRetryInterval;

	//Try to connect to the MQTT client
  if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
  	printf("C SDK - Failed to connect to MQTT, return code %d\n", rc);

		//invoke the callback so that the client is aware of the failure
		MQTTAsync_failureData* failureData = malloc(sizeof(MQTTAsync_failureData));
		failureData->token = 0;
		failureData->code = rc;
		failureData->message=(const char*)"Failed to connect to MQTT";

		_sdk_onConnectionFailure(contextWrapper, failureData);

		MQTTAsync_destroy(&client);		
    return;
  }
	
	mqttClient = client;
}

void _sdk_onSubscribeSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - _sdk_onSubscribeSuccess\n");

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;

		if (contextWrapper->onSuccess != NULL) {
			contextWrapper->onSuccess(contextWrapper->context, response);
		}
		free(contextWrapper);
	}
}

void _sdk_onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - _sdk_onSubscribeFailure\n");
	printf("C SDK - Subscribe failed, rc %d\n", response ? response->code : 0);

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;

		if (contextWrapper->onFailure != NULL) {
			contextWrapper->onFailure(contextWrapper->context, response);
		}
		free(contextWrapper);
	}
}

void subscribeCbMQTT(void* context, char *topic, int qos, CbMqttResponseOptions* options) {
	printf("C SDK - subscribeCbMQTT\n");

	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please invoke connectCbMQTT() first and then try to subscribe\n");
		return;
	}

	MQTTAsync_responseOptions responseOpts = MQTTAsync_responseOptions_initializer;
	//We have to use malloc so that the struct is allocated on the heap and not lost when the
	//function exits
	ContextWrapper* contextWrapper = (ContextWrapper*)malloc(sizeof(ContextWrapper));
	contextWrapper->context = NULL;
	contextWrapper->onSuccess = NULL;
	contextWrapper->onFailure = NULL;

	if (context != NULL) {
		contextWrapper->context = context;
	}

	//Set callbacks
	if (options != NULL) {
		if (options->onSuccess != NULL) {
			contextWrapper->onSuccess = options->onSuccess;
		}

		if (options->onFailure != NULL) {
			contextWrapper->onFailure = options->onFailure;
		}
		
		//subscribe options
		if (options->subscribeOptions != NULL && options->subscribeOptions->noLocal) responseOpts.subscribeOptions.noLocal = options->subscribeOptions->noLocal;
		if (options->subscribeOptions != NULL && options->subscribeOptions->retainAsPublished) responseOpts.subscribeOptions.retainAsPublished = options->subscribeOptions->retainAsPublished;
		if (options->subscribeOptions != NULL && options->subscribeOptions->retainHandling) responseOpts.subscribeOptions.retainHandling = options->subscribeOptions->retainHandling;
	}

	responseOpts.context = contextWrapper;
	responseOpts.onSuccess = _sdk_onSubscribeSuccess;
	responseOpts.onFailure = _sdk_onSubscribeFailure;

	int rc;
	if((rc = MQTTAsync_subscribe(mqttClient, topic, qos, &responseOpts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to subscribe to topic, return code %d\n", rc);
		return;
	}
}

void subscribeToTopic(char *topic, int qos) {
	printf("C SDK - subscribeToTopic\n");

	subscribeCbMQTT(NULL, topic, qos, NULL);
}

void _sdk_onPublishSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - _sdk_onPublishSuccess\n");

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onSuccess != NULL) {
			contextWrapper->onSuccess(contextWrapper->context, response);
		}
		free(contextWrapper);
	}
}

void _sdk_onPublishFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - _sdk_onPublishFailure\n");
	printf("C SDK - Publish failed, rc %d\n", response ? response->code : 0);

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onFailure != NULL) {
			contextWrapper->onFailure(contextWrapper->context, response);
		}
		free(contextWrapper);
	}
}

void publishCbMQTT(void* context, char *message, char *topic, int qos, int retained, CbMqttResponseOptions* options) {
	printf("C SDK - publishCbMQTT\n");
	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please invoke connectCbMQTT() first and then try to publish\n");
		return;
	}

	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	
	pubmsg.payload = message;
	pubmsg.payloadlen = strlen(message);
	pubmsg.qos = qos;
	pubmsg.retained = retained;

	//We have to use malloc so that the struct is allocated on the heap and not lost when the
	//function exits
	ContextWrapper* contextWrapper = (ContextWrapper*)malloc(sizeof(ContextWrapper));
	contextWrapper->context = NULL;
	contextWrapper->onSuccess = NULL;
	contextWrapper->onFailure = NULL;

	if (context != NULL) {
		contextWrapper->context = context;
	}

	//Set callbacks
	if (options != NULL) {
		if (options->onSuccess != NULL) {
			contextWrapper->onSuccess = options->onSuccess;
		}

		if (options->onFailure != NULL) {
			contextWrapper->onFailure = options->onFailure;
		}
	}

	opts.context = contextWrapper;
	opts.onSuccess = _sdk_onPublishSuccess;
	opts.onFailure = _sdk_onPublishFailure;

	int rc;
	if ((rc = MQTTAsync_sendMessage(mqttClient, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to publish message, return code %d\n", rc);
		return;
	}
}

void publishMessage(char *message, char *topic, int qos, int retained) {
	printf("C SDK - publishMessage\n");
	publishCbMQTT(NULL, message, topic, qos, retained, NULL);
}

void _sdk_onUnsubscribeSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - _sdk_onUnsubscribeSuccess\n");

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onSuccess != NULL) {
			contextWrapper->onSuccess(contextWrapper->context, response);
		}
		free(contextWrapper);
	}
}

void _sdk_onUnsubscribeFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - _sdk_onUnsubscribeFailure\n");
	printf("C SDK - Unsubscribe failed, rc %d\n", response ? response->code : 0);

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onFailure != NULL) {
			contextWrapper->onFailure(contextWrapper->context, response);
		}
		free(contextWrapper);
	}
}

void unsubscribeCbMQTT(void* context, char *topic, CbMqttResponseOptions* options) {
	printf("C SDK - unsubscribeCbMQTT\n");
	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please call connectCbMQTT() first and then try to unsubscribe\n");
		return;
	}

	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	//We have to use malloc so that the struct is allocated on the heap and not lost when the
	//function exits
	ContextWrapper* contextWrapper = (ContextWrapper*)malloc(sizeof(ContextWrapper));
	contextWrapper->context = NULL;
	contextWrapper->onSuccess = NULL;
	contextWrapper->onFailure = NULL;

	if (context != NULL) {
		contextWrapper->context = context;
	}

	//Set callbacks
	if (options != NULL) {
		if (options->onSuccess != NULL) {
			contextWrapper->onSuccess = options->onSuccess;
		}

		if (options->onFailure != NULL) {
			contextWrapper->onFailure = options->onFailure;
		}
	}

	opts.context = contextWrapper;
	opts.onSuccess = _sdk_onUnsubscribeSuccess;
	opts.onFailure = _sdk_onUnsubscribeFailure;

	int rc;
	if((rc = MQTTAsync_unsubscribe(mqttClient, topic, &opts)) != MQTTASYNC_SUCCESS) {
	  printf("Failed to start unsubscribe, return code %d\n", rc);
	  return;
	}
}

void unsubscribeFromTopic(char *topic) {
	printf("C SDK - unsubscribeFromTopic\n");
	unsubscribeCbMQTT(NULL, topic, NULL);
}

void _sdk_onDisconnectSuccess(void* context, MQTTAsync_successData* response) {
	printf("C SDK - _sdk_onDisconnectSuccess\n");

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onSuccess != NULL) {
			contextWrapper->onSuccess(contextWrapper->context, response);
		}
		free(contextWrapper);
		MQTTAsync_destroy(&mqttClient);
	}
}

void _sdk_onDisconnectFailure(void* context, MQTTAsync_failureData* response) {
	printf("C SDK - _sdk_onDisconnectFailure\n");
	printf("MQTT disconnect failed, rc %d\n", response ? response->code : 0);

	if (context != NULL) {
		ContextWrapper* contextWrapper = (ContextWrapper*)context;
		if (contextWrapper->onFailure != NULL) {
			contextWrapper->onFailure(contextWrapper->context, response);
		}
		free(contextWrapper);
	}
}

void disconnectMQTTClient() {
	printf("C SDK - disconnectMQTTClient\n");
	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please call connectToMQTT() first and then try to disconnect\n");
		exit(-1);
	}

	int rc;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.onSuccess = _sdk_onDisconnectSuccess;
	disc_opts.onFailure = _sdk_onDisconnectFailure;

	if((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
	        printf("Failed to start disconnect, return code %d\n", rc);
	        return;
	}
}

void disconnectCbMQTT(void* context, CbMqttDisconnectOptions *options) {
	printf("C SDK - disconnectCbMQTT\n");

	if(mqttClient == NULL) {
		printf("C SDK - You are not connected to the MQTT Broker. Please call connectCbMQTT() first and then try to disconnect\n");
		exit(-1);
	}

	int rc;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	//We have to use malloc so that the struct is allocated on the heap and not lost when the
	//function exits
	ContextWrapper* contextWrapper = (ContextWrapper*)malloc(sizeof(ContextWrapper));
	contextWrapper->context = NULL;
	contextWrapper->onSuccess = NULL;
	contextWrapper->onFailure = NULL;

	if (context != NULL) {
		contextWrapper->context = context;
	}

	disc_opts.timeout = options->timeout;

	if (options->onSuccess != NULL) {
		contextWrapper->onSuccess = options->onSuccess;
	}
	disc_opts.onSuccess = _sdk_onDisconnectSuccess;

	if (options->onFailure != NULL) {
		contextWrapper->onFailure = options->onFailure;
	}
	disc_opts.onFailure = _sdk_onDisconnectFailure;
	disc_opts.context = contextWrapper;
	
	if((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
	  printf("C SDK - Failed to start disconnect, return code %d\n", rc);
	  return;
	}
}
