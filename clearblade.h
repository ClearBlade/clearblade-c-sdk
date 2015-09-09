#ifndef _clearblade_h
#define _clearblade_h

struct ClearBlade {
	char *systemKey;
	char *systemSecret;
	char *platformURL;
	char *messagingURL;
	char *email;
	char *password;
};

void validateInitOptions(struct ClearBlade *CB);
void initialize(struct ClearBlade *CB, void callback(bool error, char *result));
void initializeClearBlade(char *systemkey, char *systemsecret, char *platformurl, char *messagingurl, char *userEmail, char *userPassword, void (*initCallback)(bool error, char *result));

#endif