#ifndef _COMMON_H
#define _COMMON_H

#define _POSIX_C_SOURCE 200809L

#include "../config.h"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

// TODO: s/error/fatal/g
#define ERROR_EPOLL_FAILED 1
#define ERROR_EPOLL_WAIT 3
#define ERROR_EPOLL_SIGNAL_MESSAGE 4
#define ERROR_SPAWN_FAILED 5
#define ERROR_EPOLL_SIGNAL 6
#define ERROR_SERVICE_SCAN 7
#define ERROR_THREAD_FAILED 8
#define ERROR_FD_FAILED 9
#define ERROR_SOCKET_FAILED 10
#define ERROR_RESPONSE 11
#define ERROR_BOOT_FAILED 12
#define ERROR_ARGS 13

#define ERROR_EXEC_FAILED 16

#ifdef TEST
#include "../tests/mock.h"
#define MOCKABLE(X) X##__real
#define __static
#else
#define MOCKABLE(X) X
#define __static static
#endif

#endif
