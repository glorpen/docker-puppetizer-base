#include "common.h"

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <sys/wait.h>

#include "service.h"
#include "spawn.h"
#include "log.h"

#define LOG_MODULE "service"

static uint8_t services_count = 0;
struct service **services;

static int service_files_filter(const struct dirent * f)
{
    int offset = strlen(f->d_name) - 6;
    if (offset > 0) {
        return strcmp(f->d_name + offset, ".start") == 0;
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

status_t service_create_all(uint8_t* count)
{
    int l_count, i;
    struct dirent **namelist;

    l_count = scandir(PUPPETIZER_SERVICE_DIR, &namelist, service_files_filter, NULL);

    if (l_count == -1) {
        log_errno_error("Searching for services in %s failed", PUPPETIZER_SERVICE_DIR);
        return S_SERVICE_COLLECT_ERROR;
    }

    services = malloc(sizeof(struct service*) * l_count);
    services_count = l_count;

    for (i=0;i<l_count; i++) {
        namelist[i]->d_name[strlen(namelist[i]->d_name)-6] = 0;
        services[i] = service_new(namelist[i]->d_name);
        log_debug("Adding service %s", services[i]->name);
        free(namelist[i]);
    }
    free(namelist);

    log_debug("Found %d services", services_count);

    if (count != NULL) {
        *count = l_count;
    }

    return S_OK;
}

void service_set_down(struct service *svc)
{
    svc->pid = 0;
    svc->state = STATE_DOWN;
}

/**
 * Runs stop script asynchronusly.
 */
bool service_stop(struct service *svc)
{
    char cmd[256];
    char pid[16];

    if (svc->state == STATE_UP) {
        log_info("Stopping service %s", svc->name);
        svc->state = STATE_PENDING_DOWN;
        cmd[snprintf(cmd, 255, PUPPETIZER_SERVICE_DIR "/%s.stop", svc->name)] = 0;
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
    int status;

    if (svc->state == STATE_DOWN) {
        log_info("Starting service %s", svc->name);
        svc->state = STATE_PENDING_UP;
        cmd[snprintf(cmd, 255, PUPPETIZER_SERVICE_DIR "/%s.start", svc->name)] = 0;
        pid_t pid = spawn2(cmd, NULL);
        if (pid > 0) {
            svc->pid = pid;

            // FIXME: will mark as failed when child somehow is SIGSTOP
            if (waitpid(pid, &status, WNOHANG) == 0) {
                svc->state = STATE_UP;
                return TRUE;
            } else {
                service_set_down(svc);
                return FALSE;
            }

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
