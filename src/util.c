/**
  * This file contains a bunch of getters and setters to set parameters related to the ClearBlade platform. 
  * Parameters include SystemKey, SystemSecret, Platform URL, Messaging URL, User Email, User Password and Auth Token/User Token
*/

#include <stdio.h>
#include "util.h"

char *USER_TOKEN = NULL;
char *SYSTEM_KEY = NULL;
char *SYSTEM_SECRET = NULL;
char *PLATFORM_URL = NULL;
char *MESSAGING_URL = NULL;
char *EMAIL = NULL;
char *PASSWORD = NULL;
char *CERT_FILE = NULL;
char *KEY_FILE = NULL;
char *DEVICE_TOKEN = NULL;

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

void setCertFile(char *certFile) {
	CERT_FILE = certFile;
}

void setKeyFile(char *keyFile) {
	KEY_FILE = keyFile;
}

void setUserToken(char *token) {
	USER_TOKEN = token;
}

void setDeviceToken(char *token) {
	DEVICE_TOKEN = token;
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

char *getCertFile() {
	return CERT_FILE;
}

char *getKeyFile() {
	return KEY_FILE;
}

char *getUserToken() {
	return USER_TOKEN;
}

char *getDeviceToken() {
	return DEVICE_TOKEN;
}
