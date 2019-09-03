#include "common.h"

#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "control.h"
#include "log.h"

#define LOG_MODULE "control"

typedef uint8_t control_header_length_t;
control_header_length_t control_max_data_length = (uint64_t)(1<<(sizeof(control_header_length_t)*8))-1;

struct init_client_t {
    int client_fd;
    struct service *svc;
};

#define MAX_SUBSCRIBED_CLIENTS 10
static struct init_client_t subscribed_clients[MAX_SUBSCRIBED_CLIENTS];

/**
 * On success returns S_OK.
 * When failed returns one of S_SOCKET_* constants.
 */
static status_t control_communicate(int fd, ssize_t (*comm)(int, void*, size_t, int), va_list ap)
{
    status_t ret = S_OK;
    control_header_length_t last_value = 0;
    uint8_t *buff, handled, len;
    int res;

    // log_debug("communicate with %d", fd);

    for(;;) {
        buff = va_arg(ap, uint8_t*);
        len = va_arg(ap, int);
        
        // log_debug("communicate: buff==null: %d, len=%d", buff == NULL, len);

        if (buff == NULL && len == 0) {
            // log_debug("eof");
            break;
        }


        // if len is not specified, use last value
        if (len == 0) {
            if (last_value == 0) {
                // log_error("No len and no last value");
                ret = S_SOCKET_ERROR;
                break;
            }
            len = last_value;
            last_value = 0;
            // log_debug("Setting len to %d", len);
        }

        if (buff == NULL) {
            if (len != sizeof(control_header_length_t)) {
                // log_error("Len field has wrong size");
                ret = S_SOCKET_ERROR;
                break;
            }
            buff = &last_value;
        }

        handled = 0;

        while (handled < len) {
            res = comm(fd, buff+handled, len-handled, 0);
            switch(res){
                case -1:
                    ret = S_SOCKET_ERROR;
                    // log_errno_warning("Socket failed");
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

        if (ret != S_OK) break;

        if (len == sizeof(control_header_length_t)) {
            last_value = *((control_header_length_t*)buff);
        }
    }
    
    return ret;
}

static status_t control_recv(int fd, ...)
{
    status_t ret;
    va_list ap;
    va_start(ap, fd);
    ret = control_communicate(fd, recv, ap);
    va_end(ap);
    return ret;
}

static status_t control_send(int fd, ...)
{
    status_t ret;
    va_list ap;
    va_start(ap, fd);
    ret = control_communicate(fd, (ssize_t (*)(int,  void *, size_t,  int))send, ap);
    va_end(ap);
    return ret;
}

/**
 * On success returns S_OK.
 * When failed returns one of S_SOCKET_* constants.
 */
status_t control_read_command(int fd, control_command_t *msg)
{
    log_debug("Reading command from %d", fd);
    return control_recv(
        fd,
        &(msg->type), sizeof(control_command_type_t),
        NULL, sizeof(control_header_length_t),
        msg->name, 0,
        NULL, 0
    );
}

/**
 * On success returns S_OK.
 * When failed returns one of S_SOCKET_* constants.
 */
status_t control_read_response(int fd, control_reponse_t* response)
{
    return control_recv(
        fd,
        response, sizeof(control_reponse_t),
        NULL, NULL
    );
}

/**
 * On success returns S_OK.
 * When failed returns one of S_SOCKET_*, S_CONTROL_* constants.
 */
status_t control_write_command(const char* name, control_command_type_t type, int fd)
{
    size_t len = strlen(name) + 1;

    if (len > control_max_data_length) {
        return S_CONTROL_SERVICE_NAME_MAXLEN;
    }

    log_debug("Writting command to %d", fd);

    return control_send(
        fd,
        &type, sizeof(control_command_type_t),
        &len, sizeof(control_header_length_t),
        name, 0,
        NULL, NULL
    );
}

status_t control_write_response(control_reponse_t response, int fd)
{
    log_debug("Sending response %d to %d", response, fd);
    return control_send(
        fd,
        &response, sizeof(control_reponse_t),
        NULL, NULL
    );
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

static status_t control_send_service_state_event(struct service* svc, struct init_client_t* client)
{
    status_t status;
    if ((status = control_write_response(CMD_RESPONSE_STATE | svc->state<<4, client->client_fd)) != S_OK) {
        log_info("Failed sending event to client");
        control_unsubscribe_client(client->client_fd);
    } else {
        log_debug("Dispatched event to client %d", client->client_fd);
    }

    return status;
}

bool control_subscribe_client(int fd, struct service *svc)
{
    log_debug("Subscribed client %d to %s", fd, svc->name);
    uint8_t i;
    for (i=0;i<MAX_SUBSCRIBED_CLIENTS;i++) {
        if (subscribed_clients[i].svc == NULL) {
            subscribed_clients[i].svc = svc;
            subscribed_clients[i].client_fd = fd;
            return control_send_service_state_event(svc, &subscribed_clients[i]) == S_OK;
        }
    }
    return false;
}

void control_unsubscribe_client(int fd)
{
    log_debug("Unsubscribed client %d", fd);
    uint8_t i;
    for (i=0;i<MAX_SUBSCRIBED_CLIENTS;i++) {
        if (subscribed_clients[i].client_fd == fd) {
            subscribed_clients[i].svc = NULL;
            subscribed_clients[i].client_fd = 0;
        }
    }
}

void control_dispatch_service_state_change(struct service *svc)
{
    uint8_t i;
    for (i=0;i<MAX_SUBSCRIBED_CLIENTS;i++) {
        if (subscribed_clients[i].svc == svc) {
            control_send_service_state_event(svc, &subscribed_clients[i]);
        }
    }
}
