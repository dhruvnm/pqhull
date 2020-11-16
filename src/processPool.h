#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include "utils.h"

// Arguments sent to the process manager thread
struct ProcManagerArgs {
    int *proc_stack, proc_num, total;
};

// Defines if quickhull is being called recursively
// or if it should wait for a message.
enum Mode {
    MESSAGE,
    RECURSIVE
};

// Various MPI message tags that serve different
// purposes. 
enum MessageTag {
    FUNC_CALL, // Message to call quickhull via message. 
    HULL_POINT, // Message to send a discovered hull point to master
    PROCESS, // Message to request or return a process
    BUF_SIZE // Message to define the buffer size needded before a FUNC_CALL
};

/******************************************************************************/

// Manages the process pool
void *processManager(void *args);

// The quickHull routine. 
void quickHull(LinkedPoint *points, int n, LinkedPoint P, LinkedPoint Q, Mode m);

#endif