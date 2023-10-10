#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "device.h"
#include "die.h"
#include "request_engine.h"
#include "util.h"
#include "user.h"
#include "clearblade.h"

struct ClearBlade CBGlobal;


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

	if (CB->email != NULL) {
		if (CB->password == NULL && CB->certFile == NULL && CB->keyFile == NULL) {
			die("Either password or certFile and keyFile must be specified when email is present");
		} else if (CB->password == NULL) {
			if (CB->certFile == NULL) {
				die("certFile must be specified when email and keyFile are present");
			} else if (CB->keyFile == NULL) {
				die("keyFile must be specified when email and certFile are present");
			}
		}
	} else if (CB->password != NULL) {
		die("Email cannot be empty when password is present");
	} else if (CB->certFile == NULL || CB->keyFile == NULL) {
		die("Email cannot be empty when certFile and keyFile are present");
	}

	setUserEmail(CB->email);

	if (CB->password != NULL) setUserPassword(CB->password);
	if (CB->certFile != NULL) setCertFile(CB->certFile);
	if (CB->keyFile != NULL) setKeyFile(CB->keyFile);
}

/**
  * This function validates the user initialize parameters and then calls the authentication function in user.c
*/
void initialize(struct ClearBlade *CB, void (*callback)(bool error, char *result)) {

	validateInitOptions(CB); // First validate all the parameters passed to the initializeClearBlade() function

	if (CB->email == NULL && CB->password == NULL) {
		authenticateAnonUser(callback); // If email and password are NULL, initialize as anonymous user
	} else {
		authenticateAuthUser(callback); // If email and password are present, initialize as authenticated user
	}
}

/**
  * This function validates the device intiailize parameters and then calls the authentication function in device.c
*/
void initializeDevice(struct ClearBlade *CB, void (*callback)(bool error, char *result)) {
	validateInitOptions(CB);

	if (CB->certFile == NULL && CB->keyFile == NULL) {
		authenticateDevice(callback);
	} else {
		authenticateDeviceX509(callback);
	}
}

/**
  * This function validates the device intiailize parameters and then calls the authentication function in device.c
*/
void initializeDeviceWithContext(void *context, struct ClearBlade *CB, void (*callback)(void *context, bool error, char *result)) {
	validateInitOptions(CB);

	if (CB->certFile == NULL && CB->keyFile == NULL) {
		authenticateDevice(callback);
	} else {
		authenticateDeviceX509(context, callback);
	}
}

/** This is one of the two first function to be called before using any of the other functions in this SDK.
  * This function initializes with the ClearBlade Platform as a system user and sets the auth token in util.c after successful initialization.
  * Except userEmail and userPassword, all other parameters are required. For Anonymous authentication pass userEmail and
  * userPassword as NULL
*/
void initializeClearBlade(char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword, void (*initCallback)(bool error, char *result)) {
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = userEmail;
	CBGlobal.password = userPassword;

	initialize(&CBGlobal, initCallback);
}

/** This is the second option of the first function to be called before using any of the other function in this SDK.
  * This funcion initializes with the ClearBlade Platform as a device within a system, and sets the auth token in util.c after successful initialization.
  * All parameters are required.
*/
void initializeClearBladeAsDevice(char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *devicename, char *activekey, void (*initCallback)(bool error, char *result)) {
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = devicename;
	CBGlobal.password = activekey;

	initializeDevice(&CBGlobal, initCallback);
}


void initializeClearBladeAsMtlsDevice(void *context, char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *devicename, char *certFile, char *keyFile, void (*initCallback)(void *context, bool error, char *result)) {
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = devicename;
	CBGlobal.certFile = certFile;
	CBGlobal.keyFile = keyFile;

	initializeDeviceWithContext(context, &CBGlobal, initCallback);
}
