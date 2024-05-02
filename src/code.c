#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "request_engine.h"
#include "concat_strings.h"
#include "json_parser.h"
#include "code.h"
#include "log.h"

/**
  * This function makes a REST call to execute the code service specified by the user and returns the response to the codeCallback
*/
void execute(char *name, char *params, void codeCallback(bool error, char *result)) {
	log_trace("C SDK - execute\n");

	char *restURLTmp1 = getConcatString("/api/v/1/code/", getSystemKey()); // REST endpoint to execute code service
	char *restURLTmp2 = getConcatString(restURLTmp1, "/");
	free(restURLTmp1);
	char *restURLTmp3 = getConcatString(restURLTmp2, name);
	free(restURLTmp2);
	char *restURL = getConcatString(getPlatformURL(), restURLTmp3); // Complete URL to make the REST call
	free(restURLTmp3);

	struct Header headers;
	memset(&headers, 0, sizeof(headers)); // Initialize headers

	headers.url = restURL;
	headers.systemKey = getSystemKey();

	if (getUserToken() != NULL) {
		headers.userToken = getUserToken();
	} else {
		headers.deviceToken = getDeviceToken();
	}
	headers.serviceName = name;	// Set headers for the REST call
	headers.body = params;
	headers.requestType = "POST";

	char *response = executeRequest(&headers); // Make the REST call

	char *result = (char *) getPropertyValueFromJson(response, "results");
	if (result == NULL)
		codeCallback(true, response); // Code Service execution unsuccessful
	else {
		codeCallback(false, result); // Code Service execution successful
	}

  	free(response);
	free(restURL);
}

/**
  * Function to execute code service without parameters. Service name and codeCallback are required parameters
*/
void executeCodeServiceWithoutParams(char *serviceName, void (*codeCallback)(bool error, char *result)) {
	log_trace("C SDK - executeCodeServiceWithoutParams\n");

	char *userAuthToken = getUserToken();
	char *deviceAuthToken = getDeviceToken();

	if (userAuthToken == NULL && deviceAuthToken == NULL) {
		codeCallback(true, "Cannot execute Code Service. Auth token is NULL. Please initialize with ClearBlade first.\n");
	} else {
		execute(serviceName, "{}", codeCallback);
	}
}

/**
  * Function to execute code service with parameters. Service name, params and codeCallback are required parameters
  * Params need to be passes as a json string
*/
void executeCodeServiceWithParams(char *serviceName, char *params, void (*codeCallback)(bool error, char *result)) {
	log_trace("C SDK - executeCodeServiceWithParams\n");

	char *userAuthToken = getUserToken();
	char *deviceAuthToken = getDeviceToken();

	if (userAuthToken == NULL && deviceAuthToken == NULL) {
		codeCallback(true, "Cannot execute Code Service. Auth token is NULL. Please initialize with ClearBlade first.\n");
	} else {
		execute(serviceName, params, codeCallback);
	}
}
