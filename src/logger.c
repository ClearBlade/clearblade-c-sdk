#include <time.h>
#include "logger.h"
#include "util.h"

void logDebug(const char* message) {
  if(strcmp(LOG_LEVEL_DEBUG, getLogLevel()) == 0) {
    log(LOG_LEVEL_DEBUG, message);
  }
}

void logInfo(const char* message) {
    if(strcmp(LOG_LEVEL_DEBUG, getLogLevel()) == 0 || strcmp(LOG_LEVEL_INFO, getLogLevel()) == 0) {
    log(LOG_LEVEL_INFO, message);
  }
}

void logError(const char* message) {
    if(strcmp(LOG_LEVEL_NONE, getLogLevel()) != 0 && strcmp(LOG_LEVEL_FATAL, getLogLevel()) != 0) {
    log(LOG_LEVEL_ERROR, message);
  }
}

void logFatal(const char* message) {
    if(strcmp(LOG_LEVEL_NONE, getLogLevel()) != 0) {
    log(LOG_LEVEL_FATAL, message);
  }
}

void log(const char* level, const char* message) {
	time_t now;
  time(&now);
  printf("%s [%s]: %s\n", ctime(&now), level, message);
}