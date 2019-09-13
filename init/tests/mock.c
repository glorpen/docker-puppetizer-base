#include "mock.h"

bool mock_init_halt_thread_executed = false;
bool mock_init_halt_thread_use = false;

void init_halt_thread(status_t cause)
{
    mock_init_halt_thread_executed = true;

    if (!mock_control_listen_use) {
        init_halt_thread__real(cause);
    }
}

bool mock_control_listen_executed = false;
bool mock_control_listen_use = false;
int mock_control_listen_fd = -1;

status_t control_listen(int* fd, uint8_t backlog)
{
    mock_control_listen_executed = true;
    
    if (mock_control_listen_use) {
        *fd = mock_control_listen_fd;
        return S_OK;
    } else {
        return control_listen__real(fd, backlog);
    }
}

bool mock_spawn2_use = false;

pid_t spawn2(const char *script, const char *arg)
{
    if (!mock_spawn2_use) {
        return spawn2__real(script, arg);
    }
}