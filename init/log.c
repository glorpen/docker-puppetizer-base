#include "common.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "log.h"

static void vlog(const char *msg, va_list ap)
{
    uint16_t len = strlen(msg);
    char tmp[len+2];
    strcpy(tmp, msg);
    strncpy(tmp+len, "\n", 2);

    vfprintf(stderr, tmp, ap);
}

static void vfatal(const char *msg, int rc, va_list ap)
{
    vlog(msg, ap);

    exit(rc);
}

void fatal(const char *msg, int rc, ...)
{
    va_list ap;
    va_start(ap, rc);
    vfatal(msg, rc, ap);
    va_end(ap);
}
void fatal_errno(const char *msg, int rc, ...)
{
    char tmp[256];
    snprintf(tmp, 255, "%s (errno=%d %s)", msg, errno, strerror(errno));

    va_list ap;
    va_start(ap, rc);
    vfatal(tmp, rc, ap);
    va_end(ap);
}

void log_debug(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vlog(msg, ap);
    va_end(ap);
}

void log_warning(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vlog(msg, ap);
    va_end(ap);
}

void log_info(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vlog(msg, ap);
    va_end(ap);
}

void log_error(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vlog(msg, ap);
    va_end(ap);
}
