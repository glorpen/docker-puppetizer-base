#ifndef _CONTROL_H
#define _CONTROL_H

#include <stdint.h>

#define SOCKET_STATUS_OK 0
#define SOCKET_STATUS_ERROR 1
#define SOCKET_STATUS_EOF 2

typedef uint8_t socket_status_t;

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

socket_status_t control_read_command(control_command_t *msg, int fd);
int control_connect();
int control_listen(uint8_t backlog);
control_reponse_t control_read_response(int socket);
socket_status_t control_write_command(int fd, const char* name, control_command_type_t type);

#endif
