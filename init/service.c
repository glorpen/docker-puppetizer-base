#include "common.h"

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>

#include "service.h"
#include "spawn.h"
#include "log.h"

static uint8_t services_count = 0;
struct service **services;

static int service_files_filter(const struct dirent * f)
{
    int offset = strlen(f->d_name) - 6;
    if (offset > 0) {
        return strcmp(f->d_name + offset, ".start") != 0;
    } else {
        return 0;
    }
}

static struct service* service_new(const char *name)
{
    struct service* svc = malloc(sizeof(struct service));
    svc->name = strdup(name);
    svc->pid = 0;
    svc->state = STATE_DOWN;

    return svc;
}

void service_create_all()
{
    int count, i;
    struct dirent **namelist;

    count = scandir(PUPPETIZER_SERVICE_DIR, &namelist, service_files_filter, NULL);

    if (count == -1) {
        fatal_errno("An error occurrend when searching for services: %s", ERROR_SERVICE_SCAN);
    }

    services = malloc(sizeof(struct service*) * count);
    services_count = count;

    for (i=0;i<count; i++) {
        services[i] = service_new(namelist[i]->d_name);
        free(namelist[i]);
    }
    free(namelist);

    log_debug("Found %d services", services_count);
}

/**
 * Runs stop script asynchronusly.
 */
bool service_stop(struct service *svc)
{
    char cmd[256];
    char pid[16];

    if (svc->state == STATE_UP) {
        svc->state = STATE_PENDING_DOWN;
        cmd[snprintf(cmd, 255, "/opt/puppetizer/services/%s.stop", svc->name)] = 0;
        sprintf(pid, "%d", svc->pid);
        // .stop powinno zrobić co się da by zatrzymać serwis "wkrótce"
        if (spawn2(cmd, pid) <= 0) {
            log_warning("Failed to run stop script for service %s", svc->name);
            svc->state = STATE_UP;
        }

        return TRUE;
    }

    return FALSE;
}

bool service_start(struct service *svc)
{
    char cmd[256];

    if (svc->state == STATE_DOWN) {
        svc->state = STATE_PENDING_UP;
        cmd[snprintf(cmd, 255, "/opt/puppetizer/services/%s.start", svc->name)] = 0;
        pid_t pid = spawn2(cmd, NULL);
        if (pid > 0) {
            svc->pid = pid;
            return TRUE;
        } else {
            log_warning("Service %s failed to start", svc->name);
            svc->state = STATE_DOWN;
        }
    }

    return FALSE;
}

uint8_t service_stop_all()
{
    uint8_t i, stopping = 0;
    for (i=0; i<services_count; i++) {
        if (services[i]->state != STATE_DOWN && services[i]->state != STATE_PENDING_DOWN) {
            service_stop(services[i]);
            stopping++;
        }
    }
    return stopping;
}

struct service* service_find_by_name(const char* name)
{
    uint8_t i;
    for (i=0; i<services_count; i++) {
        if (strcmp(name, services[i]->name) == 0) {
            return services[i];
        }
    }
    return NULL;
}

struct service* service_find_by_pid(pid_t pid)
{
    uint8_t i;
    for (i=0; i<services_count; i++) {
        if (services[i]->pid > 0 && services[i]->pid == pid) {
            return services[i];
        }
    }
    return NULL;
}

uint8_t service_count_by_state(uint8_t state, bool invert)
{
    uint8_t i, count=0;
    for (i=0; i<services_count; i++) {
        if (services[i]->state == state) {
            if(!invert) count++;
        } else {
            if(invert) count++;
        }
    }
    return count;
}
