#ifndef _device_h
#define _device_h

#include <stdbool.h>

/**
  * This function gathers all the information required to make a REST call to authenticate a device within a system
*/
void authenticateDevice(void callback(bool error, char *result));

/**
  * This function gathers all the information required to make a REST call to authenticate a device, using a x509 key pair, within a system
*/
void authenticateDeviceX509(void callback(bool error, char *result));

#endif
