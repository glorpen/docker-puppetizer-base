#include "mock.h"

bool mock_init_halt_thread_run = false;

void init_halt_thread(status_t cause)
{
    mock_init_halt_thread_run = true;
}
