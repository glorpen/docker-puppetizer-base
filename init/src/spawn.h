#ifndef _SPAWN_H
#define _SPAWN_H

#include <sys/types.h>

#define SPAWN_RETVAL_RUNNING -256

pid_t spawn1(const char *script);
pid_t spawn2(const char *script, const char *arg);
int spawn2_wait(const char *script, const char *arg);
int16_t spawn_retval(int stat);
int spawn_wait_for_pid(pid_t pid);

#endif
