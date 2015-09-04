#ifndef _util_h
#define _util_h

char *USER_TOKEN;
char *SYSTEM_KEY;
char *SYSTEM_SECRET;
char *PLATFORM_URL;
char *MESSAGING_URL;
char *EMAIL;
char *PASSWORD;

void setSystemKey(char *systemKey);
void setSystemSecret(char *systemSecret);
void setPlatformURL(char *platformURL);
void setMessagingURL(char *messagingURL);
void setUserEmail(char *email);
void setUserPassword(char *password);
void setUserToken(char *token);
char *getSystemKey();
char *getSystemSecret();
char *getPlatformURL();
char *getMessagingURL();
char *getUserEmail();
char *getUserPassword();
char *getUserToken();

#endif