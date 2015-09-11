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


/**
  * This function parses the response received from the REST call to authenticate the user to get the auth token.
*/
void parseAuthToken(char *response, void callback(bool error, char *message)) {
	char *authToken = getPropertyValueFromJson(response, "user_token"); // Gets the auth token string from the response json object
    	if (authToken == NULL)
    		callback(true, response);
    	else {
    		setUserToken(authToken); // Calls function in util.c to set the auth token
    		callback(false, authToken);
    	}
}

/**
  * This function gathers all the information required to make a REST call to authenticate the user anonymously
*/
void authenticateAnonUser(void callback(bool error, char *result)) {
	char *restURL = "/api/v/1/user/anon"; // Anonymous auth REST endpoint
	restURL = getConcatString(getPlatformURL(), restURL); // Construct complete URL for making the REST call

	struct Header headers;
	memset(&headers, 0, sizeof(headers)); // Make all elements of the Header struct to NULL

	headers.url = restURL;						
	headers.systemKey = getSystemKey();			 // Set Headers	
	headers.systemSecret = getSystemSecret();
	headers.collectionID = NULL;
	headers.requestType = "POST";
	
	char *response = executeRequest(&headers); // Make the REST Call
	parseAuthToken(response, callback);

	free(response);
	free(restURL);
}

/**
  * This function gathers all the information required to make a REST call to authenticate the user as a auth user
*/
void authenticateAuthUser(void callback(bool error, char *result)) {
	char *restEndpoint = "/api/v/1/user/auth"; // Authenticated auth REST endpoint
	char *platformurl = getPlatformURL();
	char *restURL = getConcatString(platformurl, restEndpoint); // Construct complete URL for making the REST call

	char *emailParam = getConcatString("{\"email\":\"", getUserEmail());
	char *passwordParam = getConcatString("\",\"password\":\"", getUserPassword());  // Construct POST request body with email and password of the user
	char *endBrace = "\"}";

	char *body = malloc(1 + strlen(emailParam) + strlen(passwordParam) + strlen(endBrace));
	assert(body != NULL);
	strcpy(body, emailParam);
	strcat(body, passwordParam);
	strcat(body, endBrace);

	struct Header headers;
	memset(&headers, 0, sizeof(headers)); // Make all elements of the Header struct to NULL

	headers.url = restURL;
	headers.systemKey = getSystemKey();
	headers.systemSecret = getSystemSecret();	// Set Headers	
	headers.collectionID = NULL;
	headers.body = body;
	headers.requestType = "POST";

	char *response = executeRequest(&headers); // Make the REST Call
	parseAuthToken(response, callback);

	setUserPassword(NULL); // Do not store password on the client. Make it NULL as soon as user is authenticated.

	free(response);
	free(restURL);
	free(emailParam);
	free(passwordParam);
	free(body);
}

/**
  * This function logs the user out of the ClearBlade platform. The logout callback is a required parameter and the user has to implement it
*/
void logoutUser(void (*logoutCallback)(bool error, char *result)) {
	char *restEndpoint = "/api/v/1/user/logout"; // Logout REST endpoint
	char *platformurl = getPlatformURL();
	char *restURL = getConcatString(platformurl, restEndpoint); // Construct complete URL for making the REST call

	struct Header headers;
	memset(&headers, 0, sizeof(headers)); // Make all elements of the Header struct to NULL

	headers.url = restURL;
	headers.systemKey = getSystemKey();
	headers.systemSecret = getSystemSecret(); // Set Headers
	headers.userToken = getUserToken();
	headers.requestType = "POST";

	char *response = executeRequest(&headers); // Make the REST Call

	if (strlen(response) == 0) {
		logoutCallback(false, "User logged out"); // Logout successful
	} else {
		logoutCallback(true, response); // Logout failed
	}

	free(restURL);
}

