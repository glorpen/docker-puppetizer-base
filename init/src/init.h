#ifndef _INIT_H
#define _INIT_H

#include "common.h"

int init_main(bool puppet_halt);

#define INIT_STATE_BOOTING 0
#define INIT_STATE_RUNNING 1
#define INIT_STATE_HALTING 2

#endif
