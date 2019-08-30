#include "common.h"

#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "client.h"
#include "log.h"

// TODO: move to control.c
int client_read_command(client_command_t *msg, int socket)
{
    uint8_t len = recv(socket, &(msg->type), sizeof(client_command_type_t), 0);

    if (len == 0) {
        return -1;
    }

    if (len != sizeof(client_command_type_t)) {
        return FALSE;
    }

    recv(socket, &len, sizeof(uint8_t), 0);
    recv(socket, msg->name, len, 0);
    msg->name[len] = 0;

    return TRUE;
}

client_reponse_t client_read_response(int socket)
{
    client_reponse_t ret;

    if (recv(socket, &ret, sizeof(client_reponse_t), 0) != sizeof(client_reponse_t)) {
        fatal("Partial response received", ERROR_RESPONSE);
    }

    return ret;
}

bool client_write_command(int fd, const char* name, client_command_type_t type)
{
    uint8_t len = strlen(name);
    send(fd, &type, sizeof(client_command_type_t), 0);
    send(fd, &len, sizeof(uint8_t), 0);
    send(fd, name, len, 0);

    return TRUE;
}


int client_main(int argc, char** argv)
{
    struct sockaddr_un saddr;
    socklen_t addr_size = sizeof(struct sockaddr_un);

    if (argc != 3) {
        exit(1);
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, PUPPETIZER_CONTROL_SOCKET);
    if (connect(fd, &saddr, addr_size) == -1) {
        fatal_errno("Could not connect to server.", 1);
    }

    if (strcmp(argv[1], "start") == 0) {
        if (!client_write_command(fd, argv[2], CMD_START)) {
            fatal_errno("Failed sending message", 1);
        }
        printf("response: %d\n", client_read_response(fd));
    }

    exit(0);
}
