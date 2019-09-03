#ifndef _STATUS_H
#define _STATUS_H

#include <stdint.h>

#define S_OK    0

#define S_SOCKET_ERROR 1
#define S_SOCKET_EOF   2

#define S_CONTROL_SERVICE_NAME_MAXLEN 3
#define S_CONTROL_NO_SERVER 4

#define S_SERVICE_COLLECT_ERROR 5

#define S_INIT_BOOT_FAILED 6
#define S_INIT_SERVICE_ERROR 7
#define S_INIT_HALT_REQUEST 8

typedef uint8_t status_t;
const char* status_translation(status_t status);

#endif
