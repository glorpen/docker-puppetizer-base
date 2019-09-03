#include "common.h"

#include <string.h>
#include <stdio.h>

#include "client.h"
#include "control.h"
#include "log.h"
#include "service.h"

#define LOG_MODULE "client"

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

static bool wait_for_service_state(const char *name, service_state_t target_state)
{
    control_reponse_t response;
    status_t status;

    response = client_send_message(name, CMD_SERVICE_EVENTS);

    if (! (response & CMD_RESPONSE_STATE)) {
        fatal(10, "Failed subscribing to service");
    }

    //TODO: timeout
    for (;;) {
        response = response >> 4;

        log_debug("client: is %d, wants %d", response, target_state);
        
        if (response == target_state) {
            return true;
        }

        if ((status = control_read_response(fd_control, &response)) != S_OK) {
            log_status_error(status, "asd");
            fatal(2, "Failed reading status");
        }
    }

    return false;
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

    exit(0);
}

int client_main(int argc, char** argv)
{
    status_t status;

    log_level = LOG_DEBUG;
    log_name = "client";

    if (argc != 3 && argc != 4) {
        exit(1);
    }

    status = control_connect(&fd_control);

    if (status != S_OK) {
        fatal_status(1, status, "Failed to connect to server");
    }

    if (strcmp(argv[1], "stop") == 0) {
        if (argc > 3) {
            if (client_send_message(argv[2], CMD_STOP) == CMD_RESPONSE_OK) {
                if (wait_for_service_state(argv[2], STATE_DOWN)) {
                    printf("OK\n");
                    exit(0);
                } else {
                    printf("ERROR\n");
                }
            }
        } else {
            print_response(client_send_message(argv[2], CMD_STOP));
        }
    } else if (strcmp(argv[1], "start") == 0) {
        if (argc > 3) {
            if (client_send_message(argv[2], CMD_START) == CMD_RESPONSE_OK) {
                if (wait_for_service_state(argv[2], STATE_UP)) {
                    printf("OK\n");
                    exit(0);
                } else {
                    printf("ERROR\n");
                }
            }
        } else {
            print_response(client_send_message(argv[2], CMD_START));
        }
    } else {
        print_response(client_send_message(argv[2], CMD_STATUS));
    }
    exit(1);
}
