#ifndef _CLIENT_H
#define _CLIENT_H

#include <common.h>
#include <control.h>

int client_main(const char *svc_name, control_command_type_t cmd, bool wait);

#endif
