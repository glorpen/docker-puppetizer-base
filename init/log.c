#include "common.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "log.h"

log_level_t log_level = LOG_INFO;

static const char *log_level_name(log_level_t level)
{
    switch(level) {
        case LOG_DEBUG:   return "debug";
        case LOG_ERROR:   return "error";
        case LOG_INFO:    return "info";
        case LOG_WARNING: return "warn";
        default:          return "undef";
    }
}

void vlog(log_level_t level, const char *msg, va_list ap)
{
    if (log_level < level) {
        return;
    }
    uint16_t len = strlen(msg);
    char tmp[len+2+10];
    sprintf(tmp, "[%-5s] %s\n", log_level_name(level), msg);
    vfprintf(stderr, tmp, ap);
}

void log_any(log_level_t level, const char *msg, ...)
{
        va_list ap;
        va_start(ap, msg);
        vlog(level, msg, ap);
        va_end(ap);
}

void log_status(log_level_t level, status_t status, const char *msg, ...)
{
    uint16_t size = strlen(msg) + 6 + 2 + 128; // msg + status + translation
    char tmp[size];

    snprintf(tmp, size, "%s (status=%d %s)", msg, status, status_translation(status));

    va_list ap;
    va_start(ap, msg);
    vlog(level, tmp, ap);
    va_end(ap);
}

void log_errno(log_level_t level, const char *msg, ...)
{
    uint16_t size = strlen(msg) + 5 + 2 + 128; // msg + status + translation
    char tmp[size];

    snprintf(tmp, size, "%s (errno=%d %s)", msg, errno, strerror(errno));

    va_list ap;
    va_start(ap, msg);
    vlog(level, tmp, ap);
    va_end(ap);
}
