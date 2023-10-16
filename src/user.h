#ifndef _user_h
#define _user_h

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * This function gathers all the information required to make a REST call to authenticate the user anonymously
*/
void authenticateAnonUser(void (*callback)(bool error, char *result)) __attribute__ ((deprecated));

/**
  * This function gathers all the information required to make a REST call to authenticate the user anonymously
*/
void authenticateAnonCbUser(void* context, void (*callback)(void* context, bool error, char *result));

/**
  * This function gathers all the information required to make a REST call to authenticate the user as a auth user
*/
void authenticateAuthUser(void (*callback)(bool error, char *result)) __attribute__ ((deprecated));

/**
  * This function gathers all the information required to make a REST call to authenticate the user as a auth user
*/
void authenticateAuthCbUser(void* context, void (*callback)(void* context, bool error, char *result));

/**
  * This function logs the user out of the ClearBlade platform. The logout callback is a required parameter and the user has to implement it
*/
void logoutUser(void (*logoutCallback)(bool error, char *result)) __attribute__ ((deprecated));

/**
  * This function logs the user out of the ClearBlade platform. The logout callback is a required parameter and the user has to implement it
*/
void logoutCbUser(void* context, void (*logoutCallback)(void* context, bool error, char *result));

/**
  * This function checks whether the user is authenticated
*/
void checkAuth(void (*checkAuthCallback)(bool error, char *result)) __attribute__ ((deprecated));

/**
  * This function checks whether the user is authenticated
*/
void checkCbAuth(void* context, void (*checkAuthCallback)(void* context, bool error, char *result));

#ifdef __cplusplus
}
#endif

#endif