#ifndef _TESTS_MOCK_H
#define _TESTS_MOCK_H

#include "../src/common.h"
#include "../src/status.h"

extern bool mock_init_halt_thread_executed;
extern bool mock_init_halt_thread_use;

extern bool mock_control_listen_executed;
extern bool mock_control_listen_use;
extern int mock_control_listen_fd;

void init_halt_thread(status_t cause);
void init_halt_thread__real(status_t cause);

status_t control_listen__real(int* fd, uint8_t backlog);

status_t init_loop();

#endif
