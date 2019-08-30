#include "common.h"

#include <string.h>
#include <stdio.h>

#include "client.h"
#include "control.h"
#include "log.h"

int client_main(int argc, char** argv)
{
    int fd;

    if (argc != 3) {
        exit(1);
    }

    fd = control_connect();

    if (strcmp(argv[1], "start") == 0) {
        if (control_write_command(fd, argv[2], CMD_START) != SOCKET_STATUS_OK) {
            fatal_errno("Failed sending message", 1);
        }
        printf("response: %d\n", control_read_response(fd));
    }

    exit(0);
}
