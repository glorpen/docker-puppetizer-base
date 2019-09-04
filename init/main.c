#include <argp.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "client.h"
#include "init.h"
#include "control.h"
#include "log.h"

const char *argp_program_version = "init 1.0.0";
const char *argp_program_bug_address = "<arkadiusz.dziegiel@glorpen.pl>";
static char doc[] = "Puppetizer init system.";
static char args_doc[] = "status|[<start|stop|status> <SERVICE>]";
static struct argp_option options[] = { 
    { "init", '0', 0, 0, "Run in system init mode, default if pid 1."},
    { "wait", 'w', 0, 0, "Wait for service start/stop when in client mode."},
    { "verbose", 'v', 0, 0, "Increase verbosity level (error, warning, info, debug)"},
    { 0 } 
};

struct arguments {
    enum { SERVER_MODE, CLIENT_MODE } mode;
    char *svc_name;
    service_state_t svc_action;
    bool wait;
    log_level_t log_level;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case '0': arguments->mode = SERVER_MODE; break;
        case 'c': arguments->mode = CLIENT_MODE; break;
        case 'w': arguments->wait = true; break;
        case 'v':
            if (arguments->log_level < LOG_DEBUG) {
                arguments->log_level++;
            }
            break;
        case ARGP_KEY_ARG: 
            switch (state->arg_num) {
                case 0:
                    if (strcmp(arg, "start") == 0) {
                        arguments->svc_action = CMD_START;
                    } else if (strcmp(arg, "stop") == 0) {
                        arguments->svc_action = CMD_STOP;
                    } else if (strcmp(arg, "status") == 0) {
                        arguments->svc_action = CMD_STATUS;
                    } else {
                        return ARGP_ERR_UNKNOWN;
                    }
                    break;
                case 1:
                    arguments->svc_name = arg;
                    break;
                default:
                    return ARGP_ERR_UNKNOWN;
            }
            return 0;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char** argv)
{
    struct arguments arguments;

    arguments.mode = getpid() == 1?SERVER_MODE:CLIENT_MODE;
    arguments.svc_action = CMD_STATUS;
    arguments.wait = false;
    arguments.log_level = LOG_ERROR;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    log_level = arguments.log_level;

    switch(arguments.mode) {
        case SERVER_MODE:
            log_name = "init";
            return init_main();
        case CLIENT_MODE:
            log_name = "client";
            return client_main(arguments.svc_name, arguments.svc_action, arguments.wait);
    }
}
