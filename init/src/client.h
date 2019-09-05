#ifndef _CLIENT_H
#define _CLIENT_H

#include <common.h>
#include <control.h>

#define CMD_SERVICE_START 1
#define CMD_SERVICE_STOP 2
#define CMD_SERVICE_STATUS 3
#define CMD_SERVICE_EVENTS 4
#define CMD_INIT_STATUS 5

int client_main(const char *svc_name, uint8_t cmd, bool wait);

#endif
