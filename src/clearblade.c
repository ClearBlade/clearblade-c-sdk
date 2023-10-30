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

struct ClearBladeInitParms CBGlobal;


/**
  * This function performs checks on the initialize parameters passed to the initializeClearBlade() function and validates them.
*/
void validateInitOptions(struct ClearBladeInitParms *CB) {
	printf("C SDK - validateInitOptions\n");

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

/** This is one of the two first functions to be called before using any of the other functions in this SDK.
  * This function initializes with the ClearBlade Platform as a system user and sets the auth token in util.c after successful initialization.
  * Except userEmail and userPassword, all other parameters are required. For Anonymous authentication pass userEmail and
  * userPassword as NULL
*/
void initializeClearBlade(char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword, void (*initCallback)(bool error, char *result)) {
	printf("C SDK - initializeClearBlade\n");
	
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = userEmail;
	CBGlobal.password = userPassword;

	validateInitOptions(&CBGlobal); // First validate all the parameters passed to the initializeClearBlade() function

	if (CBGlobal.email == NULL && CBGlobal.password == NULL) {
		authenticateAnonUser(initCallback); // If email and password are NULL, initialize as anonymous user
	} else {
		authenticateAuthUser(initCallback); // If email and password are present, initialize as authenticated user
	}
}

/** This is one of the two first function to be called before using any of the other functions in this SDK.
  * This function initializes with the ClearBlade Platform as a system user and sets the auth token in util.c after successful initialization.
  * Except userEmail and userPassword, all other parameters are required. For Anonymous authentication pass userEmail and
  * userPassword as NULL
*/
void cbInitialize(void *context, char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword, void (*initCallback)(void *context, bool error, char *result)) {
	printf("C SDK - initializeClearBlade\n");
	
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = userEmail;
	CBGlobal.password = userPassword;

	validateInitOptions(&CBGlobal); // First validate all the parameters passed to the initializeClearBlade() function

	if (CBGlobal.email == NULL && CBGlobal.password == NULL) {
		authenticateAnonCbUser(context, initCallback); // If email and password are NULL, initialize as anonymous user
	} else {
		authenticateAuthCbUser(context, initCallback); // If email and password are present, initialize as authenticated user
	}
}

/** This is the second option of the first function to be called before using any of the other function in this SDK.
  * This funcion initializes with the ClearBlade Platform as a device within a system, and sets the auth token in util.c after successful initialization.
  * All parameters are required.
*/
void initializeClearBladeAsDevice(char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *devicename, char *activekey, void (*initCallback)(bool error, char *result)) {
	printf("C SDK - initializeClearBladeAsDevice\n");
	
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = devicename;
	CBGlobal.password = activekey;

	validateInitOptions(&CBGlobal);
	authenticateDevice(initCallback);
}

/** This is the second option of the first function to be called before using any of the other function in this SDK.
  * This funcion initializes with the ClearBlade Platform as a device within a system, and sets the auth token in util.c after successful initialization.
  * All parameters are required.
*/
void initializeCBAsDevice(void *context, char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *devicename, char *activekey, void (*initCallback)(void *context, bool error, char *result)) {
	printf("C SDK - initializeClearBladeAsDevice\n");
	
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = devicename;
	CBGlobal.password = activekey;

	validateInitOptions(&CBGlobal);
	authenticateCbDevice(context, initCallback);
}

void initializeClearBladeAsMtlsDevice(void *context, char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *devicename, char *certFile, char *keyFile, void (*initCallback)(void *context, bool error, char *result)) {
	printf("C SDK - initializeClearBladeAsMtlsDevice\n");
	
	CBGlobal.systemKey = systemkey;
	CBGlobal.systemSecret = systemsecret;
	CBGlobal.platformURL = platformurl;
	CBGlobal.messagingURL = messagingurl;
	CBGlobal.email = devicename;
	CBGlobal.certFile = certFile;
	CBGlobal.keyFile = keyFile;

	validateInitOptions(&CBGlobal);
	if (CBGlobal.certFile == NULL && CBGlobal.keyFile == NULL) {
		authenticateCbDevice(context, initCallback);
	} else {
		authenticateDeviceX509(context, initCallback);
	}
}
