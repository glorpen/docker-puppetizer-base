#include "common.h"

#include <stdio.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "control.h"
#include "client.h"
#include "service.h"
#include "log.h"
#include "spawn.h"

#define LOG_MODULE "init"

static bool is_halting = FALSE;
static bool is_booting = FALSE;
static pid_t boot_pid;
static pthread_t halt_thread = 0;

void init_detach_from_terminal()
{
    if (ioctl(STDIN_FILENO, TIOCNOTTY) == -1) {
        log_errno_debug("Unable to detach from controlling tty");
    } else {
        /*
        * When the session leader detaches from its controlling tty via
        * TIOCNOTTY, the kernel sends SIGHUP and SIGCONT to the process
        * group (getsid(0) == getpid()).
        * But since we block all signals on start, we shouldn't receive
        * it.
        */
        
        log_debug("Detached from controlling tty");
    }
}

static pid_t init_apply()
{
    pid_t apply_pid;

    if (is_booting) {
        log_warning("Ignoring booting request");
        return 0;
    }

    is_booting = TRUE;
    apply_pid = spawn1(PUPPETIZER_APPLY);

    if (apply_pid == -1) {
        fatal(ERROR_SPAWN_FAILED, "Failed to start puppet apply");
    }
    return apply_pid;
}

bool init_handle_client_command(control_command_t *command, int socket)
{
    control_reponse_t ret = CMD_RESPONSE_ERROR;
    struct service *svc = service_find_by_name(command->name);

    if (svc == NULL) {
        log_warning("Service %s was not found", command->name);
    } else {
        log_debug("cmd type: %d", command->type);
        switch (command->type) {
            case CMD_START:
                if (is_halting) {
                    log_warning("Ignoring service start request");
                } else {
                    ret = service_start(svc)?CMD_RESPONSE_OK:CMD_RESPONSE_FAILED;
                    if (ret == CMD_RESPONSE_OK) {
                        control_dispatch_service_state_change(svc);
                    }
                }
                break;
            case CMD_STOP:
                ret = service_stop(svc)?CMD_RESPONSE_OK:CMD_RESPONSE_FAILED;
                if (ret == CMD_RESPONSE_OK) {
                    control_dispatch_service_state_change(svc);
                }
                break;
            case CMD_STATUS:
                ret = svc->state<<4 | CMD_RESPONSE_STATE;
                log_debug("resp: %d", ret);
                break;
            case CMD_SERVICE_EVENTS:
                return control_subscribe_client(socket, svc);
        }
    }

    return control_write_response(ret, socket) == S_OK;
}

/**
 * Blocks all signals.
 * SIGCHLD, SIGTERM and SIGHUP will be handled by init_loop.
 */
static void init_setup_signals()
{
    sigset_t all_signals;
    sigfillset(&all_signals);
    sigprocmask(SIG_BLOCK, &all_signals, NULL);
}

/**
 * Marks init as halting and runs shutdown process.
 * 
 * Halt process will run apply command with halt argument
 * then it tries to stop any service which is still running.
 * 
 * Will not wait for services to exit not send signal to
 * other processes.
 */
void init_halt()
{
    uint8_t i;

    if (is_halting) return;

    is_halting = TRUE;
    
    log_debug("Running halt action");
    // run puppet-apply with halt option to stop services
    int ret = spawn2_wait(PUPPETIZER_APPLY, "halt");
    if (ret != 0) {
        log_error("Puppet halt failed with exitcode %d", ret);
    }

    // stop any services that are not stopping
    i = service_stop_all();
    if (i>0) {
        log_warning("Stopping %d outstanding services.", i);
    }
    //TODO: foreach services - control_dispatch_service_state_change(svc)
}

static void init_halt_thread()
{
    int ret;

    if (halt_thread == 0) {
        log_info("Shutting down");
        ret = pthread_create(&halt_thread, NULL, (void * (*)(void *))init_halt, NULL);

        if (ret != 0) {
            fatal(ERROR_THREAD_FAILED, "Halt thread creation failed with %d", ret);
        }
    } else {
        log_warning("Tried to run halt thread multiple times.");
    }
}


static bool init_handle_signal(const struct signalfd_siginfo *info)
{
    int status;
    struct service* svc;
    service_state_t svc_state;
    int retval;

    switch(info->ssi_signo){
        case SIGCHLD:
            waitpid(info->ssi_pid, &status, WNOHANG);
            retval = spawn_retval(status);

            if (boot_pid == info->ssi_pid) {
                is_booting = FALSE;
                if (retval == 0) {
                    log_info("Booting completed");
                } else {
                    log_error("Boot script failed");
                    // failed boot script when halting is ok
                    return is_halting;
                }
            }

            svc = service_find_by_pid(info->ssi_pid);
            if (svc == NULL) {
                log_debug("Reaped PID:%d", info->ssi_pid);
            } else {
                svc_state = svc->state;
                service_set_down(svc);
                control_dispatch_service_state_change(svc);

                log_error("Service %s exitted with code %d", svc->name, retval);

                // halt if service exitted unexpectedly or it failed wither errorcode
                if (svc_state != STATE_PENDING_DOWN || retval != 0) {
                    log_debug("Service exitted with code %d when had status %d, halting", retval, svc_state);
                    init_halt_thread();
                }
            }
            break;
        case SIGTERM:
        case SIGINT:
            if (is_halting) {
                log_warning("Ignoring halting request");
            } else {
                log_debug("Halting");
                init_halt_thread();
            }
            break;
        case SIGHUP:
            log_debug("Received HUP signal");
            if (is_halting) {
                log_warning("Ignoring apply request");
            } else {
                log_debug("Running apply");
                init_apply();
            }
            break;
    }

    return TRUE;
}

static int init_create_signal_fd()
{
    int fd_signal;
    sigset_t sigmask;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGCHLD);
    sigaddset(&sigmask, SIGHUP);

    // create fd for reading required signals
    fd_signal = signalfd(-1, &sigmask, 0);
    if (fd_signal == -1) {
        fatal_errno(ERROR_FD_FAILED, "Failed to create signal descriptor");
    }

    return fd_signal;
}

static int init_loop()
{
    struct epoll_event ev, events[10];
    int changes = 0;
    uint8_t buffer[sizeof(struct signalfd_siginfo)+128];
    status_t status;

    int fd_signal, fd_epoll, fd_control, fd_client;
    uint16_t i;
    struct sockaddr_un saddr_client;
    socklen_t peer_addr_size = sizeof(struct sockaddr_un);
    bool errored;
    
    // make fd for reading required signals
    fd_signal = init_create_signal_fd();

    // make fd for control socket
    status = control_listen(&fd_control, 5);
    if (status != S_OK) {
        fatal_status(ERROR_SOCKET_FAILED, status, "Failed to create listening socket");
    }
    
    // setup epoll
    fd_epoll = epoll_create1(0);
    if (fd_epoll == -1) {
        fatal_errno(ERROR_EPOLL_FAILED, "Failed to setup polling");
    }

    ev.events = EPOLLIN;
    
    ev.data.fd = fd_signal;
    if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_signal, &ev) == -1) {
        fatal_errno(ERROR_EPOLL_FAILED, "Failed to setup signal polling");
    }
    ev.data.fd = fd_control;
    if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_control, &ev) == -1) {
        fatal_errno(ERROR_EPOLL_FAILED, "Failed to setup control socket polling");
    }
    
    for (;;) {
        changes = epoll_wait(fd_epoll, events, 10, 500);
        if (changes == -1) {
            fatal_errno(ERROR_EPOLL_WAIT, "Could not wait for events");
        }

        log_debug("loop");

        for (i = 0; i < changes; i++) {
            errored = FALSE;

            if (events[i].data.fd == fd_signal) {
                // there should be sizeof(struct signalfd_siginfo) bytes available to read
                if (read(fd_signal, buffer, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo)) {
                    log_error("Bad signal size info read");
                    return ERROR_EPOLL_SIGNAL_MESSAGE;
                }
                if (!init_handle_signal((struct signalfd_siginfo*)buffer)) {
                    return ERROR_EPOLL_SIGNAL;
                }
            }
            // handle init client
            else if (events[i].data.fd == fd_control) {
                fd_client = accept(fd_control, (struct sockaddr *) &saddr_client, &peer_addr_size);
                //setnonblocking(fd_client);
                ev.data.fd = fd_client;
                if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_client, &ev) == -1) {
                    log_errno_error("Failed to setup control client socket polling");
                    return ERROR_SOCKET_FAILED;
                }
            }
            else {
                // client connections
                status = control_read_command(events[i].data.fd, (control_command_t*)buffer);
                if (status == S_SOCKET_EOF) {
                    // socket is closed
                    log_debug("Client %d exitted", events[i].data.fd);
                    errored = TRUE;
                } else {
                    if (status == S_OK) {
                        if (!init_handle_client_command((control_command_t*)buffer, events[i].data.fd)) {
                            log_warning("Failed to handle client message from %d", events[i].data.fd);
                            errored = TRUE;
                        }
                    } else {
                        log_status_warning(status, "Failed to read client message");
                        errored = TRUE;
                    }
                }

                if (errored) {
                    ev.data.fd = events[i].data.fd;
                    if (epoll_ctl(fd_epoll, EPOLL_CTL_DEL, events[i].data.fd, &ev) == -1) {
                        fatal_errno(ERROR_EPOLL_FAILED, "Failed to remove client socket polling");
                    }
                    control_unsubscribe_client(events[i].data.fd);
                    shutdown(events[i].data.fd, SHUT_RDWR);
                }
            }
        }

        if (is_halting) {
            if (service_count_by_state(STATE_DOWN, TRUE) == 0) {
                log_info("No more services running, exitting");
                break;
            }
        }
    }

    shutdown(fd_control, SHUT_RDWR);
    unlink(PUPPETIZER_CONTROL_SOCKET);

    if (halt_thread != 0) {
        log_debug("Waiting for halt thread to exit");
        pthread_join(halt_thread, NULL);
    }
   
   return 0;
}

int init_boot()
{
    boot_pid = init_apply();
    if (boot_pid == -1) {
        fatal(ERROR_BOOT_FAILED, "Could not start boot script");
    }

    return init_loop();
}

int main(int argc, char** argv)
{
    status_t status;

    log_level = LOG_DEBUG;
    log_name = "init";

    // TODO: arg handling - loglevel, help

    if (argc == 1) {
        log_info("Running init");
        init_setup_signals();
        status = service_create_all(NULL);
        if (status != S_OK) {
            fatal_status(ERROR_BOOT_FAILED, status, "Failed to initialise services");
        }

        init_detach_from_terminal();
        return init_boot();
    } else {
        return client_main(argc, argv);
    }
}

/*
boot:
    puppet_apply
        odpala sv start nginx => /opt/puppetizer/services/nginx.start
            sv oczekuje sekundę by zobaczyć czy serwis wstał

w przyupadku gdy serwis przestanie działać, odpalamy procedurę halt z exitcode=1

sighup
    odpala puppet_apply
sigterm
    puppet_apply env=halt
        odpala sv nginx stop => /opt/puppetizer/services/nginx.stop <pid>
            init nie czeka na wyjście nginxa
            sv czeka aż stop zadziała
    gdy puppet_apply się zakończy (jakkolwiek), zostanie odpalone "sv * stop"
    init czeka aż wszystkie serwisy sie zakończą, nie przyjmuje więcej komend
    exit 0

nasłuchiwanie na sygnały, zwłaszcza SIGCHLD z waitpid nohang dla "zombie reaper"

musi też odpowiadać na sv nginx status
*/
