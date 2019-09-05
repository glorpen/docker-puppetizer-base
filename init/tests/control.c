#include <unistd.h>
#include <sys/socket.h>
#include <check.h>
#include <pthread.h>

#include "../src/common.h"
#include "../src/status.h"
#include "../src/control.h"
#include "../src/log.h"

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

START_TEST (test_name)
{
  control_response_t response_in = CMD_RESPONSE_FAILED, response_out;
  int fd[2];
  pthread_t thread;
  struct generic_read_argument th_arg;
  uint8_t data[control_max_data_length];
  // memset(data, 0, control_max_data_length);

  socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);

  th_arg.fd = fd[0];
  th_arg.data = data;
  pthread_create(&thread, NULL, generic_read, &th_arg);
  ck_assert_int_eq(control_write_response(response_in, fd[1]), S_OK);
  pthread_join(thread, NULL);
  ck_assert_int_eq(S_OK, th_arg.status);
  // data[0] = 0;
  // data[1] = 1;
  control_decode_response(data, &response_out);
  
  ck_assert_int_eq(response_in, response_out);
}
END_TEST

Suite * money_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("control communication");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_name);
    suite_add_tcase(s, tc_core);

    return s;
}


int main()
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    log_name = "test";
    log_level = LOG_DEBUG;

    s = money_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? 0 : 1;
}
