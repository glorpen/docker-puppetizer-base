#include "common.h"

#include <string.h>
#include <stdio.h>
#include <argp.h>

#include "client.h"
#include "control.h"
#include "log.h"
#include "service.h"
#include "init.h"

#define LOG_MODULE "client"

#define ASSERT(X) if(X != S_OK) { log_error("failed"); exit(1); }

int fd_control;

static control_response_t client_send_message(const char *name, service_state_t state)
{
    control_response_t response = 100;
    uint8_t data[control_max_data_length];

    ASSERT(control_set_service_state(name, state, fd_control));
    ASSERT(control_read_packet(fd_control, data));
    control_decode_response(data, &response);

    return response;
}

static bool client_set_service_state_and_wait(const char *name, service_state_t target_state)
{
    uint8_t data[control_max_data_length];
    control_response_t response;
    service_state_t state;
    
    ASSERT(control_set_service_state(name, target_state, fd_control));
    ASSERT(control_subscribe_service_state(name, fd_control));

    for (;;) {
        ASSERT(control_read_packet(fd_control, data));
        control_decode_service_state(data, &response, &state);

        log_debug("client loop");

        if (response == CMD_RESPONSE_OK) {
            if (state == target_state) {
                printf("OK\n");
                exit(0);
            }
        } else {
            printf("failed with response %d\n", response);
            exit(1);
        }
    }
}

static void print_response(control_response_t response)
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
            printf("Unknown response %d\n", response);
    }

    exit(0);
}

void cmd_init_status()
{
    uint8_t state;
    uint8_t data[control_max_data_length];

    ASSERT(control_request_init_state(fd_control));
    ASSERT(control_read_packet(fd_control, data));
    control_decode_init_state(data, &state);

    switch (state) {
        case INIT_STATE_BOOTING:
            printf("booting\n"); break;
        case INIT_STATE_RUNNING:
            printf("running\n"); break;
        case INIT_STATE_HALTING:
            printf("halting\n"); break;
        default:
            printf("unknown\n"); break;
    }

    exit(0);
}

void cmd_service_state(const char *svc_name)
{
    service_state_t state;
    control_response_t response;

    uint8_t data[control_max_data_length];
    ASSERT(control_request_service_state(svc_name, fd_control));
    ASSERT(control_read_packet(fd_control, data));

    control_decode_service_state(data, &response, &state);

    if (response != CMD_RESPONSE_OK) {
        printf("ERROR\n");
    } else {
        switch (state) {
            case STATE_UP: printf("UP\n"); break;
            case STATE_PENDING_UP: printf("PENDING UP\n"); break;
            case STATE_DOWN: printf("DOWN\n"); break;
            case STATE_PENDING_DOWN: printf("PENDING DOWN\n"); break;
            default: 
                log_warning("Unknown state: %d", state);
                printf("UNKNOWN\n");
        }
    }
}

int client_main(const char *svc_name, uint8_t cmd, bool wait)
{
    status_t status;

    status = control_connect(&fd_control);

    if (status != S_OK) {
        fatal_status(1, status, "Failed to connect to server");
    }

    switch (cmd) {
        case CMD_SERVICE_STOP:
            if (wait) {
                client_set_service_state_and_wait(svc_name, STATE_DOWN);
            } else {
                print_response(client_send_message(svc_name, STATE_DOWN));
            };
            break;
        case CMD_SERVICE_START:
            if (wait) {
                client_set_service_state_and_wait(svc_name, STATE_UP);
            } else {
                print_response(client_send_message(svc_name, STATE_UP));
            }
            break;
        case CMD_SERVICE_STATUS:
            cmd_service_state(svc_name);
            break;
        
        case CMD_INIT_STATUS:
            cmd_init_status();
            break;
    }
    exit(5);
}
