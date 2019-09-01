#include "common.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#include "spawn.h"
#include "log.h"

pid_t spawn2(const char *script, const char *arg)
{
    sigset_t no_signals;
    pid_t pid;

    const char *argv[] = {
        script,
        arg,
        0
    };
    
    pid = fork();
    if (pid < 0) {
        fatal_errno(6, "Forking failed");
    } else if (pid == 0) {
        // unblock all signals for child
        sigemptyset(&no_signals);
        sigprocmask(SIG_SETMASK, &no_signals, NULL);

        // run app
        execve(script, (char * const*)argv, NULL);
        //TODO: lepsze raportowanie do parenta?
        fatal_errno(ERROR_EXEC_FAILED, "Could not run exec for %s", script);
    }

    return pid;
}
pid_t spawn1(const char *script)
{
    return spawn2(script, NULL);
}

int spawn2_wait(const char *script, const char *arg)
{
    int stat;
    pid_t pid = spawn2(script, arg);
    
    if (pid < 0) {
        return -1;
    }
    waitpid(pid, &stat, 0);
    
    return spawn_retval(stat);
}

int16_t spawn_retval(int stat)
{
    if (WIFEXITED(stat)) {
        return WEXITSTATUS(stat);
    } else if(WIFSIGNALED(stat)) {
        // killed by signal
        return - WTERMSIG(stat);
    } else {
        // still running
        return SPAWN_RETVAL_RUNNING;
    }
}
