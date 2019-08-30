#include "common.h"

#include <string.h>
#include <stdio.h>

#include "client.h"
#include "control.h"
#include "log.h"

int fd_control;

static void service_command(const char* action, const char* want_action, const char* name, control_command_type_t type)
{
    control_reponse_t response;
    if (strcmp(action, want_action) == 0) {
        if (control_write_command(fd_control, name, type) != SOCKET_STATUS_OK) {
            fatal_errno("Failed sending message", 1);
        }
        response = control_read_response(fd_control);
        print_response(response);
        exit(0);
    }
}

void print_response(control_reponse_t response)
{
    switch(response)
    {
        case CMD_RESPONSE_OK:
            printf("OK\n"); break;
        case CMD_RESPONSE_FAILED:
            printf("failed\n"); break;
        case CMD_RESPONSE_ERROR:
            printf("an error occured\n"); break;
        default:
            if (response & CMD_RESPONSE_STATE) {
                printf("service state is %d\n", response >> 4);
            }
    }

}

int client_main(int argc, char** argv)
{
    control_reponse_t response;

    if (argc != 3) {
        exit(1);
    }

    fd_control = control_connect();

    service_command(argv[1], "start", argv[2], CMD_START);
    service_command(argv[1], "stop", argv[2], CMD_STOP);
    service_command(argv[1], "status", argv[2], CMD_STATUS);

    exit(1);
}
