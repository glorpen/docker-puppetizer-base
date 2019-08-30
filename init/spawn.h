#ifndef _SPAWN_H
#define _SPAWN_H

#include <sys/types.h>

pid_t spawn1(const char *script);
pid_t spawn2(const char *script, const char *arg);
int spawn2_wait(const char *script, const char *arg);
int8_t spawn_retval(int stat);

#endif
