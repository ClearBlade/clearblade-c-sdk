#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "json_parser.h"
#include "user.h"
#include "util.h"
#include "request_engine.h"
#include "concat_strings.h"

void parseAuthToken(char *response, void callback(bool error, char *message)) {
	char *authToken = getPropertyValueFromJson(response, "user_token");
    	if (authToken == NULL)
    		callback(true, response);
    	else {
    		setUserToken(authToken);
    		callback(false, authToken);
    	}
}

struct User *setUser(char *useremail, char *token) {
	struct User *user = malloc(sizeof(struct User));
	assert(user != NULL);

	user->email = useremail;
	user->authToken = token;

	return user;
}

void authenticateAnonUser(void callback(bool error, char *result)) {
	char *restEndpoint = "/api/v/1/user/anon";
	char *platformurl = getPlatformURL();
	char *restURL = getConcatString(platformurl, restEndpoint);

	struct Header headers;
	headers.url = restURL;
	headers.systemKey = getSystemKey();
	headers.systemSecret = getSystemSecret();
	
	char *response = executePOST(&headers);
	parseAuthToken(response, callback);

	free(response);
	free(restURL);
}

void authenticateAuthUser(void callback(bool error, char *result)) {
	char *restEndpoint = "/api/v/1/user/auth";
	char *platformurl = getPlatformURL();
	char *restURL = getConcatString(platformurl, restEndpoint);

	char *emailParam = getConcatString("{\"email\":\"", getUserEmail());
	char *passwordParam = getConcatString("\",\"password\":\"", getUserPassword());
	char *endBrace = "\"}";

	char *body = malloc(1 + strlen(emailParam) + strlen(passwordParam) + strlen(endBrace));
	assert(body != NULL);
	strcpy(body, emailParam);
	strcat(body, passwordParam);
	strcat(body, endBrace);

	struct Header headers;
	headers.url = restURL;
	headers.systemKey = getSystemKey();
	headers.systemSecret = getSystemSecret();
	headers.body = body;

	char *response = executePOST(&headers);
	parseAuthToken(response, callback);

	free(response);
	free(restURL);
	free(emailParam);
	free(passwordParam);
	free(body);
}

