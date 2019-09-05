#ifndef _SERVICE_H
#define _SERVICE_H

#include <sys/types.h>
#include "status.h"

typedef uint8_t service_state_t;

struct service {
    char* name;
    service_state_t state;
    pid_t pid;
};

#define STATE_PENDING_UP 1
#define STATE_UP 2
#define STATE_DOWN 3
#define STATE_PENDING_DOWN 4

status_t service_create_all();
uint8_t service_stop_all();
bool service_start(struct service *svc);
bool service_stop(struct service *svc);
struct service* service_find_by_name(const char* name);
struct service* service_find_by_pid(pid_t pid);
uint8_t service_count_by_state(uint8_t state, bool invert);
void service_set_down(struct service *svc);

#endif
