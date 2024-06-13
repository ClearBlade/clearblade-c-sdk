/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef CB_LOG_H
#define CB_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#define CB_LOG_VERSION "0.1.0"

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
} cb_log_Event;

typedef void (*log_LogFn)(cb_log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

enum { CB_LOG_TRACE, CB_LOG_DEBUG, CB_LOG_INFO, CB_LOG_WARN, CB_LOG_ERROR, CB_LOG_FATAL };

#define log_trace(...) cb_log_log(CB_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) cb_log_log(CB_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  cb_log_log(CB_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  cb_log_log(CB_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) cb_log_log(CB_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) cb_log_log(CB_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char* cb_log_level_string(int level);
void cb_log_set_lock(log_LockFn fn, void *udata);
void cb_log_set_level(int level);
void cb_log_set_quiet(bool enable);
int cb_log_add_callback(log_LogFn fn, void *udata, int level);
int cb_log_add_fp(FILE *fp, int level);

void cb_log_log(int level, const char *file, int line, const char *fmt, ...);

#endif