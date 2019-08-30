#include "common.h"

#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "control.h"
#include "log.h"

typedef uint8_t control_header_length_t;

static socket_status_t control_communicate(int fd, ssize_t (*comm)(int, void*, size_t, int), va_list ap)
{
    uint8_t ret = SOCKET_STATUS_OK, handled, len;
    int res;
    uint8_t *buff;
    control_header_length_t last_value = 0;

    for(;;) {
        buff = va_arg(ap, uint8_t*);
        len = va_arg(ap, int);
        if (buff == NULL && len == 0) {
            break;
        }

        // if len is not specified, use last value
        if (len == 0) {
            if (last_value == 0) {
                ret = SOCKET_STATUS_ERROR;
                break;
            }
            len = last_value;
            last_value = 0;
        }

        if (buff == NULL) {
            if (len != sizeof(control_header_length_t)) {
                ret = SOCKET_STATUS_ERROR;
                break;
            }
            buff = &last_value;
        }

        handled = 0;

        while (handled < len) {
            res = comm(fd, buff+handled, len-handled, 0);
            switch(res){
                case -1:
                    ret = SOCKET_STATUS_ERROR;
                    break;
                case 0:
                    ret = SOCKET_STATUS_EOF;
                    break;
                default:
                    handled += res;
                    break;
            }

            if (ret != SOCKET_STATUS_OK) break;
        }

        if (ret != SOCKET_STATUS_OK) break;

        if (len == sizeof(control_header_length_t)) {
            last_value = *((control_header_length_t*)buff);
        }
    }
    
    return ret;
}

static socket_status_t control_recv(int fd, ...)
{
    socket_status_t ret;
    va_list ap;
    va_start(ap, fd);
    ret = control_communicate(fd, recv, ap);
    va_end(ap);
    return ret;
}

static socket_status_t control_send(int fd, ...)
{
    socket_status_t ret;
    va_list ap;
    va_start(ap, fd);
    ret = control_communicate(fd, send, ap);
    va_end(ap);
    return ret;
}

socket_status_t control_read_command(control_command_t *msg, int fd)
{
    return control_recv(
        fd,
        &(msg->type), sizeof(control_command_type_t),
        NULL, sizeof(control_header_length_t),
        msg->name, 0,
        NULL, 0
    );
}

control_reponse_t control_read_response(int fd)
{
    control_reponse_t response;
    socket_status_t status = control_recv(
        fd,
        &response, sizeof(control_reponse_t)
    );

    if (status != SOCKET_STATUS_OK) {
        fatal("Partial response received", ERROR_RESPONSE);
    }

    return response;
}

socket_status_t control_write_command(int fd, const char* name, control_command_type_t type)
{
    control_header_length_t len = strlen(name) + 1;
    return control_send(
        fd,
        &type, sizeof(control_command_type_t),
        &len, sizeof(control_header_length_t),
        name, 0,
        NULL, NULL
    );
}

int control_connect()
{
    struct sockaddr_un saddr;
    socklen_t addr_size = sizeof(struct sockaddr_un);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, PUPPETIZER_CONTROL_SOCKET);
    if (connect(fd, (const struct sockaddr *)&saddr, addr_size) == -1) {
        fatal_errno("Could not connect to server.", 1);
    }

    return fd;
}

int control_listen(uint8_t backlog)
{
    struct sockaddr_un saddr_server;

    int fd_control = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_control == -1) {
        fatal_errno("Failed to create control socket", ERROR_SOCKET_FAILED);
    }
    memset(&saddr_server, 0, sizeof(struct sockaddr_un));
    saddr_server.sun_family = AF_UNIX;
    strcpy(saddr_server.sun_path, PUPPETIZER_CONTROL_SOCKET);

    if (bind(fd_control, (struct sockaddr *) &saddr_server, sizeof(struct sockaddr_un)) == -1) {
        fatal_errno("Failed to bind control socket", ERROR_SOCKET_FAILED);
    }

    if (listen(fd_control, backlog) == -1) {
        fatal_errno("Failed to listen on control socket", ERROR_SOCKET_FAILED);
    }

    return fd_control;
}
