#ifndef _clearblade_h
#define _clearblade_h

#include <pthread.h>
#include <stdbool.h>
#include "messaging.h"
#include "code.h"
#include "query.h"
#include "user.h"


/**
  * The ClearBlade struct containing the initialize parameters.
  * This is just to make things simple and understand what is going on during initialization
*/
struct ClearBlade {
	char *systemKey;
	char *systemSecret;
	char *platformURL;
	char *messagingURL;
	char *email;
	char *password;
	pthread_mutex_t connectLock;
};

extern struct ClearBlade CBGlobal;



/** This is the first function to be called before using any of the other functions in this SDK.
  * This function initializes with the ClearBlade Platform and sets the auth token in util.c after successful initialization.
  * Except userEmail and userPassword, all other parameters are required. For Anonymous authentication pass userEmail and
  * userPassword as NULL
*/
void initializeClearBlade(char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword, void (*initCallback)(bool error, char *result));
void initializeClearBladeSync (char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword);

#endif
