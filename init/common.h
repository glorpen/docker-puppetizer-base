#ifndef _COMMON_H
#define _COMMON_H

#define _POSIX_C_SOURCE 200809L

// #define PUPPETIZER_SERVICE_DIR "/opt/puppetizer/services"
#define PUPPETIZER_SERVICE_DIR "/mnt/sandbox/workspace/glorpen-docker/doc_puppetizer_base/.local/init/services"
// #define PUPPETIZER_APPLY "/opt/puppetizer/bin/apply"
#define PUPPETIZER_APPLY "/mnt/sandbox/workspace/glorpen-docker/doc_puppetizer_base/.local/init/apply"
#define PUPPETIZER_CONTROL_SOCKET "/mnt/sandbox/workspace/glorpen-docker/doc_puppetizer_base/.local/init/control.socket"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#define TRUE 1
#define FALSE 0
typedef uint8_t bool;

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

#endif
