#include <stdio.h>
#include "util.h"

char *USER_TOKEN = NULL;
char *SYSTEM_KEY = NULL;
char *SYSTEM_SECRET = NULL;
char *PLATFORM_URL = NULL;
char *MESSAGING_URL = NULL;
char *EMAIL = NULL;
char *PASSWORD = NULL;

void setSystemKey(char *systemKey) {
	SYSTEM_KEY = systemKey;
}

void setSystemSecret(char *systemSecret) {
	SYSTEM_SECRET = systemSecret;
}

void setPlatformURL(char *platformURL) {
	PLATFORM_URL = platformURL;
}

void setMessagingURL(char *messagingURL) {
	MESSAGING_URL = messagingURL;
}

void setUserEmail(char *email) {
	EMAIL = email;
}

void setUserPassword(char *password) {
	PASSWORD = password;
}

void setUserToken(char *token) {
	USER_TOKEN = token;
}

char *getSystemKey() {
	return SYSTEM_KEY;
}

char *getSystemSecret() {
	return SYSTEM_SECRET;
}

char *getPlatformURL() {
	return PLATFORM_URL;
}

char *getMessagingURL() {
	return MESSAGING_URL;
}

char *getUserEmail() {
	return EMAIL;
}

char *getUserPassword() {
	return PASSWORD;
}

char *getUserToken() {
	return USER_TOKEN;
}