#ifndef _user_h
#define _user_h

struct User {
	char *email;
	char *authToken;
};

void parseAuthToken(char *response, void callback(bool error, char *message));
void authenticateAnonUser(void callback(bool error, char *result));
void authenticateAuthUser(void callback(bool error, char *result));

#endif