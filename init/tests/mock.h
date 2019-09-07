#ifndef _TESTS_MOCK_H
#define _TESTS_MOCK_H

#include "../src/common.h"
#include "../src/status.h"

extern bool mock_init_halt_thread_run;
void init_halt_thread(status_t cause);

#endif
