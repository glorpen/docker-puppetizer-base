#include "common.h"

#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "control.h"
#include "log.h"

#define LOG_MODULE "control"

typedef uint8_t control_header_length_t;
const uint8_t control_max_data_length = (uint64_t)(1<<(sizeof(control_header_length_t)*8))-1;

struct init_client_t {
    int client_fd;
    struct service *svc;
};

#define MAX_SUBSCRIBED_CLIENTS 10
static struct init_client_t subscribed_clients[MAX_SUBSCRIBED_CLIENTS];

#define PACKET_FIRST_DATA(X) (((uint8_t*)X)+sizeof(control_type_t))

static size_t control_memcpy(void *dst, const void *src, size_t len)
{
    memcpy(dst, src, len);
    return len;
}

static status_t control_communicate(int fd,  ssize_t (*comm)(int, void*, size_t, int), control_header_length_t len, void *data)
{
    control_header_length_t handled = 0;
    status_t ret = S_OK;
    int res;
    // const char* method = comm == recv?"recv":"send";

    while (handled < len) {
        // log_debug("%s, handled: %d, len: %d", method, handled, len);
        res = comm(fd, ((uint8_t*)data)+handled, len-handled, 0);
        switch(res){
            case -1:
                ret = S_SOCKET_ERROR;
                log_errno_error("Socket communcation failed");
                break;
            case 0:
                ret = S_SOCKET_EOF;
                break;
            default:
                handled += res;
                break;
        }

        if (ret != S_OK) break;
    }

    return ret;
}

status_t control_read_packet(int fd, void *data)
{
    control_header_length_t len;
    status_t status;

    log_debug("Reading len packet from %d", fd);
    
    status = control_communicate(fd, recv, sizeof(control_header_length_t), &len);
    if (status != S_OK) {
        return status;
    }

    log_debug("Reading %d bytes from %d", len, fd);

    return control_communicate(fd, recv, len, data);
}

static status_t control_write_packet(int fd, control_type_t type, control_header_length_t len, const void *data)
{
    uint8_t payload_size = len + sizeof(control_type_t);
    uint16_t packet_size = sizeof(control_header_length_t) + payload_size;
    uint8_t buff[packet_size];
    uint8_t *p = buff;

    p += control_memcpy(p, &payload_size, sizeof(control_header_length_t));
    p += control_memcpy(p, &type, sizeof(control_type_t));
    p += control_memcpy(p, data, len);
    
    log_debug("Writting %d bytes to %d", packet_size, fd);

    return control_communicate(fd, (ssize_t (*)(int,  void *, size_t,  int))send, packet_size, buff);
}

/**
 * On success returns S_OK.
 * When failed returns one of S_SOCKET_*, S_CONTROL_* constants.
 */
status_t control_set_service_state(const char* name, service_state_t state, int fd)
{
    log_debug("sending set_service_state");
    size_t len = strlen(name) + 1 + sizeof(uint8_t);
    uint8_t buff[control_max_data_length];
    uint8_t *p = buff;

    p += control_memcpy(p, &state, sizeof(service_state_t));
    memcpy(p, name, strlen(name)+1);

    return control_write_packet(fd, PACKET_SET_SERVICE_STATE, len, buff);
}

void control_decode_set_service_state(void *packet, char **svc_name, service_state_t *state)
{
    uint8_t *p = PACKET_FIRST_DATA(packet);
    p += control_memcpy(state, p, sizeof(service_state_t));
    *svc_name = (char*)p;
}

status_t control_request_service_state(const char* name, int fd)
{
    log_debug("sending request_service_state");
    return control_write_packet(fd, PACKET_REQUEST_SERVICE_STATE, strlen(name) + 1, name);
}
void control_decode_request_service_state(void *packet, char **svc_name)
{
    *svc_name = (char*)PACKET_FIRST_DATA(packet);
}
status_t control_write_service_state(control_response_t response, service_state_t state, int fd)
{
    control_header_length_t len = sizeof(control_response_t) + sizeof(service_state_t);
    uint8_t buff[len];
    uint8_t *p = buff;

    p += control_memcpy(p, &response, sizeof(control_response_t));
    control_memcpy(p, &state, sizeof(service_state_t));

    return control_write_packet(fd, PACKET_SERVICE_STATE, len, buff);
}
void control_decode_service_state(void *packet, control_response_t *response, service_state_t *state)
{
    uint8_t *p = PACKET_FIRST_DATA(packet);

    p += control_memcpy(response, p, sizeof(control_response_t));
    control_memcpy(state, p, sizeof(service_state_t));
}

status_t control_subscribe_service_state(const char* name, int fd)
{
    return control_write_packet(fd, PACKET_SUBSCRIBE_SERVICE_STATE, strlen(name) + 1, name);
}
void control_decode_subscribe_service_state(void *packet, char **svc_name)
{
    control_decode_request_service_state(packet, svc_name);
}

status_t control_write_response(control_response_t response, int fd)
{
    return control_write_packet(fd, PACKET_COMMAND_RESPONSE, sizeof(control_response_t), &response);
}
void control_decode_response(void *packet, control_response_t *response)
{
    control_memcpy(response, PACKET_FIRST_DATA(packet), sizeof(control_response_t));
}
status_t control_request_init_state(int fd)
{
    return control_write_packet(fd, PACKET_REQUEST_INIT_STATE, 0, NULL);
}

status_t control_write_init_state(uint8_t state, int fd)
{
    return control_write_packet(fd, PACKET_INIT_STATE, sizeof(uint8_t), &state);
}
void control_decode_init_state(void *packet, uint8_t *state)
{
    control_memcpy(state, PACKET_FIRST_DATA(packet), sizeof(uint8_t));
}


status_t control_connect(int* fd)
{
    struct sockaddr_un saddr;
    socklen_t addr_size = sizeof(struct sockaddr_un);

    *fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, PUPPETIZER_CONTROL_SOCKET);
    if (connect(*fd, (const struct sockaddr *)&saddr, addr_size) == -1) {
        return S_CONTROL_NO_SERVER;
    }

    return S_OK;
}

status_t control_listen(int* fd, uint8_t backlog)
{
    struct sockaddr_un saddr_server;

    int fd_control = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_control == -1) {
        return S_SOCKET_ERROR;
        // fatal_errno("Failed to create control socket", ERROR_SOCKET_FAILED);
    }
    memset(&saddr_server, 0, sizeof(struct sockaddr_un));
    saddr_server.sun_family = AF_UNIX;
    strcpy(saddr_server.sun_path, PUPPETIZER_CONTROL_SOCKET);

    if (bind(fd_control, (struct sockaddr *) &saddr_server, sizeof(struct sockaddr_un)) == -1) {
        return S_SOCKET_ERROR;
        // fatal_errno("Failed to bind control socket", ERROR_SOCKET_FAILED);
    }

    if (listen(fd_control, backlog) == -1) {
        return S_SOCKET_ERROR;
        // fatal_errno("Failed to listen on control socket", ERROR_SOCKET_FAILED);
    }

    *fd = fd_control;
    return S_OK;
}

bool control_subscribe_client(int fd, struct service *svc)
{
    log_debug("Subscribed client %d to %s", fd, svc->name);
    uint8_t i;
    for (i=0;i<MAX_SUBSCRIBED_CLIENTS;i++) {
        if (subscribed_clients[i].svc == NULL) {
            subscribed_clients[i].svc = svc;
            subscribed_clients[i].client_fd = fd;
            return control_write_service_state(CMD_RESPONSE_OK, svc->state, subscribed_clients[i].client_fd) == S_OK;
        }
    }
    return false;
}

void control_unsubscribe_client(int fd)
{
    uint8_t i;
    bool found = false;

    for (i=0;i<MAX_SUBSCRIBED_CLIENTS;i++) {
        if (subscribed_clients[i].client_fd == fd) {
            subscribed_clients[i].svc = NULL;
            subscribed_clients[i].client_fd = 0;
            found = true;
        }
    }
    if (found) {
        log_debug("Unsubscribed client %d", fd);
    }
}

void control_dispatch_service_state_change(struct service *svc)
{
    uint8_t i;
    for (i=0;i<MAX_SUBSCRIBED_CLIENTS;i++) {
        if (subscribed_clients[i].svc == svc) {
            control_write_service_state(CMD_RESPONSE_OK, svc->state, subscribed_clients[i].client_fd);
        }
    }
}
