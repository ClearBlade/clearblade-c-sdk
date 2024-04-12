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
#include "logger.h"


/**
  * This function parses the response received from the REST call to authenticate the user to get the auth token.
*/
__attribute__ ((deprecated))
void parseAuthToken(char *response, void callback(bool error, char *message)) {
	logDebug("C SDK - parseAuthToken\n");

	char *authToken = (char *) getPropertyValueFromJson(response, "user_token"); // Gets the auth token string from the response json object
    	if (authToken == NULL)
    		callback(true, response);
    	else {
    		setUserToken(authToken); // Calls function in util.c to set the auth token
    		callback(false, authToken);
    	}
}

/**
  * This function parses the response received from the REST call to authenticate the user to get the auth token.
*/
void parseCbAuthToken(void* context, char *response, void callback(void* context, bool error, char *message)) {
	logDebug("C SDK - parseAuthToken\n");

	char *authToken = (char *) getPropertyValueFromJson(response, "user_token"); // Gets the auth token string from the response json object
    	if (authToken == NULL)
    		callback(context, true, response);
    	else {
    		setUserToken(authToken); // Calls function in util.c to set the auth token
    		callback(context, false, authToken);
    	}
}

/**
  * This function gathers all the information required to make a REST call to authenticate the user anonymously
*/
__attribute__ ((deprecated))
void authenticateAnonUser(void (*callback)(bool error, char *result)) {
	logDebug("C SDK - authenticateAnonUser\n");

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
  * This function gathers all the information required to make a REST call to authenticate the user anonymously
*/
void authenticateAnonCbUser(void* context, void (*callback)(void* context, bool error, char *result)) {
	logDebug("C SDK - authenticateAnonCbUser\n");

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
	parseCbAuthToken(context, response, callback);

	free(response);
	free(restURL);
}

/**
  * This function gathers all the information required to make a REST call to authenticate the user as a auth user
*/
__attribute__ ((deprecated))
void authenticateAuthUser(void (*callback)(bool error, char *result)) {
	logDebug("C SDK - authenticateAuthUser\n");

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

	//setUserPassword(NULL); // Do not store password on the client. Make it NULL as soon as user is authenticated.

	free(response);
	free(restURL);
	free(emailParam);
	free(passwordParam);
	free(body);
}

/**
  * This function gathers all the information required to make a REST call to authenticate the user as a auth user
*/
void authenticateAuthCbUser(void* context, void (*callback)(void* context, bool error, char *result)) {
	logDebug("C SDK - authenticateAuthCbUser\n");

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
	parseCbAuthToken(context, response, callback);

	//setUserPassword(NULL); // Do not store password on the client. Make it NULL as soon as user is authenticated.

	free(response);
	free(restURL);
	free(emailParam);
	free(passwordParam);
	free(body);
}

/**
  * This function logs the user out of the ClearBlade platform. The logout callback is a required parameter and the user has to implement it
*/
__attribute__ ((deprecated))
void logoutUser(void (*logoutCallback)(bool error, char *result)) {
	logDebug("C SDK - logoutUser\n");

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

/**
  * This function logs the user out of the ClearBlade platform. The logout callback is a required parameter and the user has to implement it
*/
void logoutCbUser(void* context, void (*logoutCallback)(void* context, bool error, char *result)) {
	logDebug("C SDK - logoutUser\n");

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
		logoutCallback(context, false, "User logged out"); // Logout successful
	} else {
		logoutCallback(context, true, response); // Logout failed
	}

	free(restURL);
}

/**
  * This function checks whether the user is authenticated
*/
__attribute__ ((deprecated))
void checkAuth(void (*checkAuthCallback)(bool error, char *result)) {
	logDebug("C SDK - checkAuth\n");

	char *restEndpoint = "/api/v/1/user/checkauth"; // Checkauth REST endpoint
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

	char *authResponse = (char *) getPropertyValueFromJson(response, "is_authenticated");
    	if (authResponse == NULL)
    		checkAuthCallback(false, response);
    	else {
    		checkAuthCallback(true, authResponse);
    	}

	free(restURL);
}

/**
  * This function checks whether the user is authenticated
*/
void checkCbAuth(void* context, void (*checkAuthCallback)(void* context, bool error, char *result)) {
	logDebug("C SDK - checkAuth\n");

	char *restEndpoint = "/api/v/1/user/checkauth"; // Checkauth REST endpoint
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

	char *authResponse = (char *) getPropertyValueFromJson(response, "is_authenticated");
    	if (authResponse == NULL)
    		checkAuthCallback(context, false, response);
    	else {
    		checkAuthCallback(context, true, authResponse);
    	}

	free(restURL);
}
