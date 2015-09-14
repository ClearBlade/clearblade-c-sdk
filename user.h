#ifndef _user_h
#define _user_h


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