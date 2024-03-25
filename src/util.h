/**
  * This file contains a bunch of getters and setters to set parameters related to the ClearBlade platform. 
  * Parameters include SystemKey, SystemSecret, Platform URL, Messaging URL, User Email, User Password and Auth Token/User Token
*/

#ifndef _util_h
#define _util_h

extern char *USER_TOKEN;
extern char *DEVICE_TOKEN;
extern char *SYSTEM_KEY;
extern char *SYSTEM_SECRET;
extern char *PLATFORM_URL;
extern char *MESSAGING_URL;
extern char *EMAIL;
extern char *PASSWORD;

void setSystemKey(char *systemKey);
void setSystemSecret(char *systemSecret);
void setPlatformURL(char *platformURL);
void setMessagingURL(char *messagingURL);
void setUserEmail(char *email);
void setUserPassword(char *password);
void setCertFile(char *certFile);
void setKeyFile(char *keyFile);
void setUserToken(char *token);
void setDeviceToken(char *token);
char *getSystemKey();
char *getSystemSecret();
char *getPlatformURL();
char *getMessagingURL();
char *getUserEmail();
char *getUserPassword();
char *getCertFile();
char *getKeyFile();
char *getUserToken();
char *getDeviceToken();

#endif
