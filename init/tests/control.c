#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

#include "control.h"

#include "../src/common.h"
#include "../src/status.h"
#include "../src/control.h"

struct generic_read_argument {
  int fd;
  void *data;
  status_t status;
};
void* generic_read(void *data)
{
  struct generic_read_argument *arg = data;
  arg->status = control_read_packet(arg->fd, arg->data);

  return NULL;
}

START_TEST (test_communication)
{
  control_response_t response_in = CMD_RESPONSE_FAILED, response_out;
  int fd[2];
  pthread_t thread;
  struct generic_read_argument th_arg;
  uint8_t data[control_max_data_length];

  socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);

  th_arg.fd = fd[0];
  th_arg.data = data;
  pthread_create(&thread, NULL, generic_read, &th_arg);
  ck_assert_int_eq(control_write_response(response_in, fd[1]), S_OK);
  pthread_join(thread, NULL);
  ck_assert_int_eq(S_OK, th_arg.status);
  control_decode_response(data, &response_out);
  
  ck_assert_int_eq(response_in, response_out);
}
END_TEST

TCase * tcontrol_create_test_case(void)
{
    TCase *tc;

    tc = tcase_create("Control");

    tcase_add_test(tc, test_communication);

    return tc;
}
