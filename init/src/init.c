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

#include "init.h"
#include "control.h"
#include "service.h"
#include "log.h"
#include "spawn.h"

#define LOG_MODULE "init"

static bool is_halting = false;
static bool is_booting = false;
static pid_t boot_pid;
static pthread_t halt_thread = 0;
static status_t halt_cause = S_OK;
static bool use_puppet_when_halting = false;

static void init_detach_from_terminal()
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

    is_booting = true;
    apply_pid = spawn1(PUPPETIZER_APPLY);

    if (apply_pid == -1) {
        fatal(ERROR_SPAWN_FAILED, "Failed to start puppet apply");
    }
    return apply_pid;
}

static uint8_t init_get_state()
{
    if (is_booting) {
        return INIT_STATE_BOOTING;
    }
    if (is_halting) {
        return INIT_STATE_HALTING;
    }
    return INIT_STATE_RUNNING;
}

static status_t init_handle_client_command(void *packet, int fd)
{
    struct service *svc;
    char *svc_name;
    service_state_t svc_state;
    control_response_t response;
    bool ret;

    switch (PACKET_TYPE(packet)) {
        
        case PACKET_REQUEST_SERVICE_STATE:
            log_debug("Handling request for service state for %d", fd);
            control_decode_request_service_state(packet, &svc_name);
            svc = service_find_by_name(svc_name);
            if (svc == NULL) {
                return control_write_service_state(CMD_RESPONSE_FAILED, 0, fd);
            } else {
                return control_write_service_state(CMD_RESPONSE_OK, svc->state, fd);
            }
        
        case PACKET_SET_SERVICE_STATE:
            control_decode_set_service_state(packet, &svc_name, &svc_state);
            svc = service_find_by_name(svc_name);
            log_debug("Handling setting service state for %d and service %s", fd, svc_name);

            if (svc == NULL) {
                response = CMD_RESPONSE_ERROR;
            } else {
                switch (svc_state) {
                    case STATE_UP:
                        response = service_start(svc)?CMD_RESPONSE_OK:CMD_RESPONSE_FAILED;
                        break;
                    case STATE_DOWN:
                        response = service_stop(svc)?CMD_RESPONSE_OK:CMD_RESPONSE_FAILED;
                        break;
                    default:
                        log_error("Bad service state %d from client %d", svc_state, fd);
                        response = CMD_RESPONSE_ERROR;
                }
            }
            return control_write_response(response, fd);
        
        case PACKET_SUBSCRIBE_SERVICE_STATE:
            log_debug("Handling service event subscribing for %d", fd);
            control_decode_subscribe_service_state(packet, &svc_name);
            svc = service_find_by_name(svc_name);

            if (svc == NULL) {
                log_warning("Client %d tried to subscribe to nonexisting service %s", fd, svc_name);
                response = CMD_RESPONSE_ERROR;
            } else {
                ret = control_subscribe_client(fd, svc);
                if (!ret) {
                    response = CMD_RESPONSE_ERROR;
                } else {
                    response = CMD_RESPONSE_OK;
                }
            }

            if (response != CMD_RESPONSE_OK) {
                return control_write_service_state(response, 0, fd);
            } else {
                return S_OK;
            }
        case PACKET_REQUEST_INIT_STATE:
            log_debug("Handling request for init state for %d", fd);
            return control_write_init_state(init_get_state(), fd);
        default:
            log_error("Unknown packet %d", PACKET_TYPE(packet));
            return S_UNKNOWN_ERROR;
    }
}

/**
 * Blocks all signals.
 * SIGCHLD, SIGTERM and SIGHUP will be handled by init_loop.
 */
__static void init_setup_signals()
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
static void init_halt()
{
    uint8_t i;
    int ret;

    if (is_halting) return;

    is_halting = true;
    
    log_debug("Running halt action");

    if (use_puppet_when_halting) {
        // run puppet-apply with halt option to stop services
        ret = spawn2_wait(PUPPETIZER_APPLY, "halt");
        if (ret != 0) {
            log_error("Puppet halt failed with exitcode %d", ret);
        }
    }

    // stop any services that are not stopping
    i = service_stop_all();
    if (i>0) {
        log_warning("Stopping %d outstanding services.", i);
    }
}

__static void MOCKABLE(init_halt_thread)(status_t cause)
{
    int ret;

    if (halt_thread == 0 && !is_halting) {
        halt_cause = cause;
        log_info("Halting init");
        ret = pthread_create(&halt_thread, NULL, (void * (*)(void *))init_halt, NULL);

        if (ret != 0) {
            fatal(ERROR_THREAD_FAILED, "Halt thread creation failed with %d", ret);
        }
    }
}


static void init_handle_signal(const struct signalfd_siginfo *info)
{
    int status;
    struct service* svc;
    service_state_t svc_state;
    int retval;
    pid_t pid;

    log_debug("Handling signal");

    // multiple signals do not stack, wait for all processes
    for (;;) {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) {
            break;
        }

        retval = spawn_retval(status);

        if (boot_pid == pid) {
            is_booting = false;
            if (retval == 0) {
                log_info("Booting completed");
            } else {
                log_error("Boot script failed");
                init_halt_thread(S_INIT_BOOT_FAILED);
            }
        }

        svc = service_find_by_pid(pid);
        if (svc == NULL) {
            log_debug("Reaped PID:%d", pid);
        } else {
            svc_state = svc->state;
            service_set_down(svc);

            log_error("Service %s exitted with code %d", svc->name, retval);

            // halt if service exitted unexpectedly or it failed wither errorcode
            if (svc_state != STATE_PENDING_DOWN || retval != 0) {
                log_debug("Service exitted with code %d when had status %d, halting", retval, svc_state);
                init_halt_thread(S_INIT_SERVICE_ERROR);
            }
        }
    }

    switch(info->ssi_signo){
        case SIGCHLD:
            break;
        case SIGTERM:
        case SIGINT:
            log_debug("Received TERM/INT signal");
            init_halt_thread(S_INIT_HALT_REQUEST);
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

__static status_t init_loop()
{
    struct epoll_event ev, events[10];
    int changes = 0;
    uint8_t buffer[sizeof(struct signalfd_siginfo)+128];
    status_t status;
    uint8_t packet[control_max_data_length];

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

    // TODO: fatal_* should be replaced with S_INIT_* as it is return code to init
    
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
            errored = false;

            if (events[i].data.fd == fd_signal) {
                // there should be sizeof(struct signalfd_siginfo) bytes available to read
                if (read(fd_signal, buffer, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo)) {
                    log_error("Bad signal size info read");
                    return ERROR_EPOLL_SIGNAL_MESSAGE;
                }
                init_handle_signal((struct signalfd_siginfo*)buffer);
            }
            // handle init client
            else if (events[i].data.fd == fd_control) {
                fd_client = accept(fd_control, (struct sockaddr *) &saddr_client, &peer_addr_size);
                ev.data.fd = fd_client;
                if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_client, &ev) == -1) {
                    log_errno_error("Failed to setup control client socket polling");
                    return ERROR_SOCKET_FAILED;
                }
            }
            else {
                // client connections
                status = control_read_packet(events[i].data.fd, packet);
                if (status == S_SOCKET_EOF) {
                    // socket is closed
                    log_debug("Client %d exitted", events[i].data.fd);
                    errored = true;
                } else {
                    if (status == S_OK) {
                        if (init_handle_client_command(packet, events[i].data.fd) != S_OK) {
                            log_warning("Failed to handle client message from %d", events[i].data.fd);
                            errored = true;
                        }
                    } else {
                        log_status_warning(status, "Failed to read client message");
                        errored = true;
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
            if (service_count_by_state(STATE_DOWN, true) == 0) {
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
   
   return halt_cause;
}

static int init_boot()
{
    boot_pid = init_apply();
    if (boot_pid == -1) {
        fatal(ERROR_BOOT_FAILED, "Could not start boot script");
    }

    return init_loop();
}

int init_main(bool puppet_halt)
{
    status_t status;

    use_puppet_when_halting = puppet_halt;

    log_info("Running init");
    init_setup_signals();
    status = service_create_all(NULL);
    if (status != S_OK) {
        fatal_status(ERROR_BOOT_FAILED, status, "Failed to initialise services");
    }

    init_detach_from_terminal();
    return init_boot();
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
