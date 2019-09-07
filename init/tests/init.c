#include "../src/common.h"

#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "init.h"

#include "../src/status.h"

static void* run_loop_signals(void *data)
{
  init_loop();
  return NULL;
}

/**
 * Test case for handling signals when multiple child exits at same time.
 */
START_TEST (test_signals_in_loop)
{
  int fd[2], i;
  pid_t pid;
  pthread_t thread;
  sigset_t all_signals;

  socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);

  mock_init_halt_thread_use = true;
  mock_control_listen_use = true;
  mock_control_listen_fd = fd[1];

  sigfillset(&all_signals);
  sigprocmask(SIG_BLOCK, &all_signals, NULL);

  pthread_create(&thread, NULL, run_loop_signals, NULL);

  for (i=0;i<5;i++) {
    if (fork() == 0) {
      sleep(1);
      exit(0);
    }
  }

  sleep(2);

  shutdown(fd[1], SHUT_RDWR);
  pthread_join(thread, NULL);

  pid = waitpid(-1, NULL, WNOHANG);
  ck_assert_msg(pid <= 0, "All child processes should be handled");
}
END_TEST

TCase * tinit_create_test_case(void)
{
    TCase *tc;

    tc = tcase_create("Init");

    tcase_add_test(tc, test_signals_in_loop);

    return tc;
}
