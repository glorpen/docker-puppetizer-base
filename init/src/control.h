#ifndef _CONTROL_H
#define _CONTROL_H

#include "status.h"
#include "service.h"

#define PACKET_SET_SERVICE_STATE 1
#define PACKET_COMMAND_RESPONSE 2
#define PACKET_REQUEST_SERVICE_STATE 3
#define PACKET_SUBSCRIBE_SERVICE_STATE 4
#define PACKET_SERVICE_STATE 5
#define PACKET_REQUEST_INIT_STATE 6
#define PACKET_INIT_STATE 7

#define CMD_RESPONSE_ERROR 0
#define CMD_RESPONSE_OK 1
#define CMD_RESPONSE_FAILED 2

extern const uint8_t control_max_data_length;

// typedef uint8_t control_command_type_t;
// typedef struct control_command_t {
//     control_command_type_t type;
//     char name[];
// } control_command_t;

typedef uint8_t control_response_t;
typedef uint8_t control_type_t;

// status_t control_read_command(int fd, control_command_t *msg);
status_t control_connect(int* fd);
status_t control_listen(int *fd, uint8_t backlog);
// status_t control_read_response(int fd, control_reponse_t* response, uint8_t *payload);
// status_t control_write_command(const char* name, control_command_type_t type, int fd);
// status_t control_write_response(control_reponse_t response, uint8_t payload, int fd);

bool control_subscribe_client(int fd, struct service *svc);
void control_unsubscribe_client(int fd);
void control_dispatch_service_state_change(struct service *svc);

status_t control_read_packet(int fd, void *data);

status_t control_set_service_state(const char* name, service_state_t type, int fd);
void control_decode_set_service_state(void *packet, char **svc_name, service_state_t *state);

status_t control_request_service_state(const char* name, int fd);
void control_decode_request_service_state(void *packet, char **svc_name);

status_t control_write_response(control_response_t response, int fd);
void control_decode_response(void *packet, control_response_t *response);

status_t control_write_service_state(control_response_t response, service_state_t state, int fd);
void control_decode_service_state(void *packet, control_response_t *response, service_state_t *state);

status_t control_subscribe_service_state(const char* name, int fd);
void control_decode_subscribe_service_state(void *packet, char **svc_name);

status_t control_request_init_state(int fd);
status_t control_write_init_state(uint8_t state, int fd);
void control_decode_init_state(void *packet, uint8_t *state);


#define PACKET_TYPE(X) ((control_type_t*)X)[0]

#endif
