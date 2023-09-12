/**
  * This file contains a bunch of getters and setters to set parameters related to the ClearBlade platform. 
  * Parameters include SystemKey, SystemSecret, Platform URL, Messaging URL, User Email, User Password and Auth Token/User Token
*/

#ifndef _util_h
#define _util_h

char *USER_TOKEN;
char *DEVICE_TOKEN;
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
void setCertFilePath(char *certFilePath);
void setKeyFilePath(char *keyFilePath);
void setUserToken(char *token);
void setDeviceToken(char *token);
char *getSystemKey();
char *getSystemSecret();
char *getPlatformURL();
char *getMessagingURL();
char *getUserEmail();
char *getUserPassword();
char *getCertFilePath();
char *getKeyFilePath();
char *getUserToken();
char *getDeviceToken();

#endif
