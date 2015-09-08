#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "die.h"
#include "request_engine.h"
#include "util.h"
#include "user.h"
#include "code.h"

struct ClearBlade {
	char *systemKey;
	char *systemSecret;
	char *platformURL;
	char *messagingURL;
	char *email;
	char *password;
};

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

void initialize(struct ClearBlade *CB, void callback(bool error, char *result)) {

	validateInitOptions(CB);

	if (CB->email == NULL && CB->password == NULL) {
		authenticateAnonUser(callback);
	} else {
		authenticateAuthUser(callback);
	}
	
	free(CB);
}

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

void logoutCallback(bool error, char *message) {
	if (!error) {
		printf("Logout: %s\n", message);
	}
}

void codeCallback(bool error, char *message) {
	if (error) {
		printf("ERROR: %s\n", message);
	} else {
		printf("SUCCESS: %s\n", message);
	}
}

void initCallback(bool error, char *message) {
	if (error) {
		printf("ERROR: %s\n", message);
	} else {
		printf("SUCCESS: %s\n", message);
	}
}

int main(int argc, char *argv[]) {
	initializeClearBlade("bccba9de0a94e4daef8ed189aff801", "BCCBA9DE0AF2BAF39FB1EFB0CE47", "https://rtp.clearblade.com", "rtp.clearblade.com:1883", "pi@clearblade.com", "clearblade", initCallback);
	executeCodeServiceWithoutParams("test", codeCallback);
	logoutUser(logoutCallback);

	return 0;
}