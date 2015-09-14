#ifndef _user_h
#define _user_h


/**
  * This function parses the response received from the REST call to authenticate the user to get the auth token.
*/
void parseAuthToken(char *response, void callback(bool error, char *message));


/**
  * This function gathers all the information required to make a REST call to authenticate the user anonymously
*/
void authenticateAnonUser(void callback(bool error, char *result));


/**
  * This function gathers all the information required to make a REST call to authenticate the user as a auth user
*/
void authenticateAuthUser(void callback(bool error, char *result));


/**
  * This function logs the user out of the ClearBlade platform. The logout callback is a required parameter and the user has to implement it
*/
void logoutUser(void (*logoutCallback)(bool error, char *result));


/**
  * This function checks whether the user is authenticated
*/
void checkAuth(void (*checkAuthCallback)(bool error, char *result));

#endif