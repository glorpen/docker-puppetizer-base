#ifndef _LOG_H
#define _LOG_H

#include <stdarg.h>
#include "status.h"

#define LOG_ERROR 0
#define LOG_WARNING 1
#define LOG_INFO 2
#define LOG_DEBUG 3
typedef uint8_t log_level_t;

extern log_level_t log_level;
extern char *log_name;

void log_any(log_level_t level, const char *module, const char *msg, ...);
void log_status(log_level_t level, const char *module, status_t status, const char *msg, ...);
void log_errno(log_level_t level, const char *module, const char *msg, ...);

#define log_error(...) log_any(LOG_ERROR, LOG_MODULE, __VA_ARGS__)
#define log_info(...) log_any(LOG_INFO, LOG_MODULE, __VA_ARGS__)
#define log_warning(...) log_any(LOG_WARNING, LOG_MODULE, __VA_ARGS__)
#define log_debug(...) log_any(LOG_DEBUG, LOG_MODULE, __VA_ARGS__)

#define log_status_error(...) log_status(LOG_ERROR, LOG_MODULE, __VA_ARGS__)
#define log_status_info(...) log_status(LOG_INFO, LOG_MODULE, __VA_ARGS__)
#define log_status_warning(...) log_status(LOG_WARNING, LOG_MODULE, __VA_ARGS__)
#define log_status_debug(...) log_status(LOG_DEBUG, LOG_MODULE, __VA_ARGS__)

#define log_errno_error(...) log_errno(LOG_ERROR, LOG_MODULE, __VA_ARGS__)
#define log_errno_info(...) log_errno(LOG_INFO, LOG_MODULE, __VA_ARGS__)
#define log_errno_warning(...) log_errno(LOG_WARNING, LOG_MODULE, __VA_ARGS__)
#define log_errno_debug(...) log_errno(LOG_DEBUG, LOG_MODULE, __VA_ARGS__)

#define fatal(RC, ...) log_error(__VA_ARGS__); exit(RC)
#define fatal_errno(RC, ...) log_errno_error(__VA_ARGS__); exit(RC)
#define fatal_status(RC, ...) log_status_error(__VA_ARGS__); exit(RC)

#endif
