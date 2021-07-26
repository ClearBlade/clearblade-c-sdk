#ifndef _device_h
#define _device_h

#include <stdbool.h>

/**
  * This function gathers all the information required to make a REST call to authenticate a device within a system
*/
void authenticateDevice(void callback(bool error, char *result));

#endif
