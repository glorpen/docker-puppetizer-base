#ifndef _CLIENT_H
#define _CLIENT_H

#include <stdint.h>

#define CMD_START 1
#define CMD_STOP 2
#define CMD_STATUS 3
#define CMD_STOP_BLOCK 4

#define CMD_RESPONSE_ERROR 0
#define CMD_RESPONSE_OK 1
#define CMD_RESPONSE_FAILED 2
#define CMD_RESPONSE_STATE 3

typedef uint8_t client_command_type_t;

typedef struct client_command_t {
    client_command_type_t type;
    char name[];
} client_command_t;

typedef uint8_t client_reponse_t;

int client_main(int argc, char** argv);
int client_read_command(client_command_t *msg, int socket);

#endif
