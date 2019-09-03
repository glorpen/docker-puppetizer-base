#include "common.h"

#include <string.h>
#include <stdio.h>

#include "client.h"
#include "control.h"
#include "log.h"
#include "service.h"

int fd_control;

static const char *translate_service_state(service_state_t state)
{
    switch (state) {
        case STATE_UP: return "UP";
        case STATE_PENDING_UP: return "PENDING UP";
        case STATE_DOWN: return "DOWN";
        case STATE_PENDING_DOWN: return "PENDING DOWN";
        default: 
            log_warning("Unknown state: %d", state);
            return "UNKNOWN";
    }
}

static control_reponse_t client_send_message(const char *name, control_command_type_t type)
{
    status_t status;
    control_reponse_t response;

    if ((status = control_write_command(name, type, fd_control)) != S_OK) {
        fatal(11, "Failed sending message");
    }
    if ((status = control_read_response(fd_control, &response)) != S_OK) {
        fatal(12, "Failed reading response");
    }

    return response;
}

static service_state_t client_service_status(const char *name)
{
    return client_send_message(name, CMD_STATUS) >> 4;
}

static bool wait_for_service_state(const char *name, service_state_t target_state)
{
    control_reponse_t response;
    status_t status;

    if (response = client_send_message(name, CMD_SERVICE_EVENTS) != CMD_RESPONSE_OK) {
        fatal(10, "Failed subscribing to service");
    }

    //TODO: timeout
    for (;;) {
        if (control_read_response(fd_control, &response) != S_OK) {
            fatal(2, "Failed reading status");
        }
        response = response >> 4;

        log_debug("client: is %d, wants %d", response, target_state);
        //TODO: check if pending up => up, pending => down or pending up => down (failed)
        if (response == target_state) {
            return TRUE;
        }
    }

    return FALSE;
}

static void print_response(control_reponse_t response)
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
                printf("%s\n", translate_service_state(response >> 4));
            } else {
                printf("Unknown response\n");
            }
    }
}

static void service_command(const char* action, const char* want_action, const char* name, control_command_type_t type)
{
    status_t status;
    control_reponse_t response;

    if (strcmp(action, want_action) == 0) {
        if ((status = control_write_command(name, type, fd_control)) != S_OK) {
            fatal(3, "Failed sending message");
        }
        if ((status = control_read_response(fd_control, &response)) != S_OK) {
            fatal(4, "Failed reading response");
        }
        print_response(response);
        exit(0);
    }
}

int client_main(int argc, char** argv)
{
    status_t status;

    if (argc != 3 && argc != 4) {
        exit(1);
    }

    status = control_connect(&fd_control);

    if (status != S_OK) {
        fatal_status(1, status, "Failed to connect to server");
    }

    // if (strcmp(argv[1], "start") == 0) {
    //     print_response(client_send_message(argv[2], CMD_START));
    //     wait_for_service_state(argv[2], STATE_UP);
    // }

    if (strcmp(argv[1], "stop") == 0) {
        log_debug("####### argc: %d", argc);
        if (argc > 3) {
            if (client_send_message(argv[2], CMD_STOP) == CMD_RESPONSE_OK) {
                if (wait_for_service_state(argv[2], STATE_DOWN)) {
                    printf("OK\n");
                    exit(0);
                } else {
                    printf("ERROR\n");
                }
            }
            exit(1);
        } else {
            service_command(argv[1], "stop", argv[2], CMD_STOP);
        }
    } else {
        service_command(argv[1], "start", argv[2], CMD_START);
        service_command(argv[1], "status", argv[2], CMD_STATUS);
    }
    exit(1);
}
