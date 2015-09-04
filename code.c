#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "util.h"
#include "request_engine.h"
#include "concat_strings.h"
#include "code.h"

void execute(char *name, char *params, void codeCallback(bool error, char *result)) {
	char *temp = getConcatString("/api/v/1/code/", getSystemKey());
	char *tempEndpoint = getConcatString(temp, "/");
	char *restEndpoint = getConcatString(tempEndpoint, name);
	char *platformurl = getPlatformURL();
	char *restURL = getConcatString(platformurl, restEndpoint);
	
	struct Header headers;
	headers.url = restURL;
	headers.systemKey = getSystemKey();
	headers.userToken = getUserToken();
	headers.serviceName = name;
	headers.body = params;

	char *response = executePOST(&headers);
	codeCallback(false, response);

	free(tempEndpoint);
	free(temp);
	free(restEndpoint);
	free(restURL);
}

void executeCodeServiceWithoutParams(char *serviceName, void (*codeCallback)(bool error, char *result)) {
	char *authToken = getUserToken();

	if (authToken == NULL) {
		codeCallback(true, "Cannot execute Code Service. Auth token is NULL. Please initialize with ClearBlade first.\n");
	} else {
		execute(serviceName, "{}", codeCallback);
	}
} 

void executeCodeServiceWithParams(char *serviceName, char *params, void (*codeCallback)(bool error, char *result)) {

}