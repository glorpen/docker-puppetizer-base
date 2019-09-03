#ifndef _CONTROL_H
#define _CONTROL_H

#include "status.h"
#include "service.h"

#define CMD_START 1
#define CMD_STOP 2
#define CMD_STATUS 3
#define CMD_SERVICE_EVENTS 4

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
status_t control_write_response(control_reponse_t response, int fd);

bool control_subscribe_client(int fd, struct service *svc);
void control_unsubscribe_client(int fd);
void control_dispatch_service_state_change(struct service *svc);

#endif
