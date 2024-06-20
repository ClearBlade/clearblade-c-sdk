#include <time.h>
#include <string.h>
#include <stdio.h>
#include "logger.h"
#include "util.h"

void logMessage(const char* level, const char* message) {
	time_t now;
  time(&now);
  printf("%s [%s]: %s\n", ctime(&now), level, message);
}

void logDebug(const char* message) {
  if(strcmp(LOG_LEVEL_DEBUG, getLogLevel()) == 0) {
    logMessage(LOG_LEVEL_DEBUG, message);
  }
}

void logInfo(const char* message) {
    if(strcmp(LOG_LEVEL_DEBUG, getLogLevel()) == 0 || strcmp(LOG_LEVEL_INFO, getLogLevel()) == 0) {
    logMessage(LOG_LEVEL_INFO, message);
  }
}

void logError(const char* message) {
    if(strcmp(LOG_LEVEL_NONE, getLogLevel()) != 0 && strcmp(LOG_LEVEL_FATAL, getLogLevel()) != 0) {
    logMessage(LOG_LEVEL_ERROR, message);
  }
}

void logFatal(const char* message) {
    if(strcmp(LOG_LEVEL_NONE, getLogLevel()) != 0) {
    logMessage(LOG_LEVEL_FATAL, message);
  }
}