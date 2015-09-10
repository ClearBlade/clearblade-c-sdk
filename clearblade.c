#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "die.h"
#include "request_engine.h"
#include "util.h"
#include "user.h"
#include "clearblade.h"


/**
  * This function performs checks on the initialize parameters passed to the initializeClearBlade() function and validates them.
*/
void validateInitOptions(struct ClearBlade *CB) {
	if (CB->systemKey == NULL) {
		die("SystemKey cannot be empty");
	} else if (CB->systemSecret == NULL) {
		die("SystemSecret cannot be empty");
	} else {
		setSystemKey(CB->systemKey);
		setSystemSecret(CB->systemSecret);
	}

	if (CB->platformURL == NULL) {
		die("PlatformURL cannot be empty");
	} else {
		setPlatformURL(CB->platformURL);
	}

	if (CB->messagingURL == NULL) {
		die("MessagingURL cannot be empty");
	} else {
		setMessagingURL(CB->messagingURL);
	}

	if (CB->email != NULL && CB->password == NULL) {
		die("Password cannot be empty when email is present");
	} else if (CB->password != NULL && CB->email == NULL) {
		die("Email cannot be empty when password is present");
	} else {
		setUserEmail(CB->email);
		setUserPassword(CB->password);
	}
}

/**
  * This function validates the initialize parameters and then calls the authentication function in user.c
*/
void initialize(struct ClearBlade *CB, void callback(bool error, char *result)) {

	validateInitOptions(CB); // First validate all the parameters passed to the initializeClearBlade() function

	if (CB->email == NULL && CB->password == NULL) {
		authenticateAnonUser(callback); // If email and password are NULL, initialize as anonymous user
	} else {
		authenticateAuthUser(callback); // If email and password are present, initialize as authenticated user
	}
	
	free(CB); // cleanup the ClearBlade struct after use
}

/** This is the first function to be called before using any of the other functions in this SDK.
  * This function initializes with the ClearBlade Platform and sets the auth token in util.c after successful initialization.
  * Except userEmail and userPassword, all other parameters are required. For Anonymous authentication pass userEmail and 
  * userPassword as NULL
*/
void initializeClearBlade(char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword, void (*initCallback)(bool error, char *result)) {
	struct ClearBlade *CB = malloc(sizeof(struct ClearBlade));
	assert(CB != NULL);

	CB->systemKey = systemkey;
	CB->systemSecret = systemsecret;
	CB->platformURL = platformurl;
	CB->messagingURL = messagingurl;
	CB->email = userEmail;
	CB->password = userPassword;

	initialize(CB, initCallback);
}
