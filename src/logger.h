#ifndef _logger_h
#define _logger_h

extern const char *LOG_LEVEL_DEBUG = "DEBUG";
extern const char *LOG_LEVEL_INFO = "INFO";
extern const char *LOG_LEVEL_ERROR = "ERROR";
extern const char *LOG_LEVEL_FATAL = "FATAL";
extern const char *LOG_LEVEL_NONE = "NONE";

void logDebug(const char* message);
void logInfo(const char* message);
void logError(const char* message);
void logFatal(const char* message);

#endif