#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "json_parser.h"
#include "device.h"
#include "util.h"
#include "request_engine.h"
#include "concat_strings.h"

void authenticateDevice(void callback(bool error, char *result)) {

	char *restURLTmp1 = getConcatString("/api/v/2/devices/", getSystemKey());
	char *restURLTmp2 = getConcatString(restURLTmp1, "/auth");
	free(restURLTmp1);
	char *restURL = getConcatString(getPlatformURL(), restURLTmp2);
	free(restURLTmp2);

	char *deviceNameParam = getConcatString("{\"deviceName\":\"", getUserEmail());
	char *activeKeyParam = getConcatString("\",\"activeKey\":\"", getUserPassword());
	char *endBrace = "\"}";

	char *body = malloc(1 + strlen(deviceNameParam) + strlen(activeKeyParam) + strlen(endBrace));
	assert(body != NULL);
	strcpy(body, deviceNameParam);
	strcat(body, activeKeyParam);
	strcat(body, endBrace);

	struct Header headers;
	memset(&headers, 0, sizeof(headers));

	headers.url = restURL;
	headers.systemKey = getSystemKey();
	headers.systemSecret = getSystemSecret();
	headers.collectionID = NULL;
	headers.body = body;
	headers.requestType = "POST";

	char *response = executeRequest(&headers);

	char *authToken = (char *) getPropertyValueFromJson(response, "deviceToken");
	if (authToken == NULL) {
		callback(true, response);
	} else {
		setDeviceToken(authToken);
		callback(false, authToken);
	}

	free(response);
	free(restURL);
	free(deviceNameParam);
	free(activeKeyParam);
	free(body);
}

void authenticateDeviceX509(void callback(bool error, char *result)) {
	char *restURL = ":444/api/v/4/devices/mtls/auth";
	restURL = getConcatString(getPlatformURL(), restURL);

	char *deviceNameParam = getConcatString("{\"name\":\"", getUserEmail());
	char *systemKeyParam = getConcatString("\",\"system_key\":\"", getSystemKey());
	char *endBrace = "\"}";

	char *body = malloc(1 + strlen(deviceNameParam) + strlen(systemKeyParam) + strlen(endBrace));
	assert(body != NULL);
	strcpy(body, deviceNameParam);
	strcat(body, systemKeyParam);
	strcat(body, endBrace);

	struct Header headers;
	memset(&headers, 0, sizeof(headers));

	headers.url = restURL;
	headers.systemKey = getSystemKey();
	headers.systemSecret = getSystemSecret();
	headers.collectionID = NULL;
	headers.body = body;
	headers.requestType = "POST";

	char *response = executex509MtlsRequest(&headers, getCertFile(), getKeyFile());

	char *authToken = (char *) getPropertyValueFromJson(response, "deviceToken");
	if (authToken == NULL) {
		callback(true, response);
	} else {
		setDeviceToken(authToken);
		callback(false, authToken);
	}

	free(response);
	free(restURL);
	free(deviceNameParam);
	free(systemKeyParam);
	free(body);
}