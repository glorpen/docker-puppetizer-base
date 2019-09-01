#ifndef _CONTROL_H
#define _CONTROL_H

#include "status.h"

#define CMD_START 1
#define CMD_STOP 2
#define CMD_STATUS 3
#define CMD_STOP_BLOCK 4

#define CMD_RESPONSE_ERROR 0
#define CMD_RESPONSE_OK 1
#define CMD_RESPONSE_FAILED 2
#define CMD_RESPONSE_STATE 3

typedef uint8_t control_command_type_t;

typedef struct control_command_t {
    control_command_type_t type;
    char name[];
} control_command_t;

typedef uint8_t control_reponse_t;

status_t control_read_command(int fd, control_command_t *msg);
status_t control_connect(int* fd);
status_t control_listen(int *fd, uint8_t backlog);
status_t control_read_response(int fd, control_reponse_t* response);
status_t control_write_command(const char* name, control_command_type_t type, int fd);

#endif
