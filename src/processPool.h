#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include "utils.h"

struct ProcManagerArgs {
    int *proc_stack, proc_num, total;
};

enum Mode {
    MESSAGE,
    RECURSIVE
};

enum MessageTag {
    FUNC_CALL,
    HULL_POINT,
    PROCESS
};

/******************************************************************************/

// Manages the process pool
void *processManager(void *args);

void quickHull(LinkedPoint *points, int n, LinkedPoint P, LinkedPoint Q, Mode m);

#endif