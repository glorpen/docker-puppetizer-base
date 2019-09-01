#ifndef _LOG_H
#define _LOG_H

#include <stdarg.h>
#include "status.h"

void fatal(const char *msg, int rc, ...);
void fatal_errno(const char *msg, int rc, ...);
void log_error(const char *msg, ...);
void log_info(const char *msg, ...);
void log_warning(const char *msg, ...);
void log_debug(const char *msg, ...);

#endif
