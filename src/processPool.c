#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"

#include "utils.h"
#include "processPool.h"

MPI_Datatype MPI_LINKED_POINT;

int main(int argc, char **argv) {
    // Initialize MPI
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    define_MPI_LINKED_POINT(&MPI_LINKED_POINT);

    // Master Process
    if (rank == 0) {
        if (size < 3) {
            printf("This program requires at least 3 processes\n");
            exit(1);
        }

        struct Arguments arg;
        struct Point* points;
        struct LinkedPoint *hull, *P, *Q, *top, *bot, h_idxs[3];
        int i, t, b, *proc_stack, proc_num, total, top_proc, bot_proc, h_count;
        pthread_t thread_id;
        struct ProcManagerArgs args;
        MPI_Request top_req, bot_req, hull_req;
        MPI_Status hull_stat;

        parseArgs(argc, argv, &arg);
        if (arg.numPoints <= 0) {
            printf("NumPoints %d must be larger than 0\n", arg.numPoints);
            exit(1);
        } else if (arg.inFile == NULL) {
            printf("Infile is undefined\n");
            exit(1);
        } else if (arg.outFile == NULL) {
            printf("Outfile is undefined\n");
            exit(1);
        }

        points = (Point *)malloc(arg.numPoints * sizeof(struct Point));
        loadFile(arg.inFile, points, arg.numPoints);

        // Convert list of points to linked points
        hull = (LinkedPoint *)malloc(arg.numPoints * sizeof(struct LinkedPoint));
        for (i = 0; i < arg.numPoints; i++) {
            hull[i].point = points[i];
            hull[i].index = i;
            hull[i].prev = NULL;
            hull[i].next = NULL;
        }

        free(points);

        // START TIMING

        // Find the leftmost and rightmost points
        P = &hull[0]; // leftmost
        Q = &hull[0]; // rightmost
        for (i = 1; i < arg.numPoints; i++) {
            if (hull[i].point.x < (*P).point.x) {
                P = &hull[i];
            }

            if (hull[i].point.x > (*Q).point.x) {
                Q = &hull[i];
            }
        }

        // Leftmost and rightmost form our initial hull
        P->next = Q;
        P->prev = Q;
        Q->next = P;
        Q->prev = P;

        // Find the points on top and below the line formed by P and Q
        top = (LinkedPoint *)malloc(arg.numPoints * sizeof(struct LinkedPoint));
        bot = (LinkedPoint *)malloc(arg.numPoints * sizeof(struct LinkedPoint));
        t = 0;
        b = 0;
        for (i = 0; i < arg.numPoints; i++) {
            if (findSide(P->point, Q->point, hull[i].point) > 0) {
                top[t++] = hull[i];
            } else {
                bot[b++] = hull[i];
            }
        }

        // Set up process manager
        proc_num = size - 1;
        total = size - 1;
        proc_stack = (int *)malloc(total * sizeof(int));
        for (i = 0; i < total; i++) {
            proc_stack[i] = i + 1;
        }

        // Get two process from the stack
        top_proc = proc_stack[proc_num - 1];
        bot_proc = proc_stack[proc_num - 2];
        proc_num -= 2;

        // Start process manager
        args.proc_stack = proc_stack;
        args.proc_num = proc_num;
        args.total = total;
        pthread_create(&thread_id, NULL, processManager, (void *)&args);

        // Call QuickHull
        MPI_Isend(top, t, MPI_LINKED_POINT, top_proc, FUNC_CALL, MPI_COMM_WORLD, &top_req);
        MPI_Isend(bot, b, MPI_LINKED_POINT, bot_proc, FUNC_CALL, MPI_COMM_WORLD, &bot_req);

        while (true) {
            MPI_Irecv(&h_idxs, 3, MPI_INT, MPI_ANY_SOURCE, HULL_POINT, MPI_COMM_WORLD, &hull_req);
            MPI_Wait(&hull_req, &hull_stat);
            MPI_Get_count(&hull_stat, MPI_INT, &h_count);

            if (h_count == 0) {
                // Stopping condition
                break;
            }

            // Otherwise we have a point to insert into the hull
            hull[h_idxs[0]].next = &hull[h_idxs[1]];
            hull[h_idxs[1]].prev = &hull[h_idxs[0]];
            hull[h_idxs[1]].next = &hull[h_idxs[2]];
            hull[h_idxs[2]].prev = &hull[h_idxs[1]];
        }

        pthread_join(thread_id, NULL);

        writePointListToFile(arg.outFile, P);
    } else {
        // Worker processes
        LinkedPoint P; // dummy point to pass as an arg
        quickHull(NULL, 0, P, P, MESSAGE);
    }
    return 0;
}

void *processManager(void *args) {
    ProcManagerArgs *arguments = (ProcManagerArgs *)args;

    int i, *proc_stack, proc_num, total, ret_proc, p_count;
    MPI_Request proc_req;
    MPI_Status proc_stat;
    proc_stack = arguments->proc_stack;
    proc_num = arguments->proc_num;
    total = arguments->total;

    while (proc_num != total) {
        MPI_Irecv(&ret_proc, 1, MPI_INT, MPI_ANY_SOURCE, PROCESS, MPI_COMM_WORLD, &proc_req);
        MPI_Wait(&proc_req, &proc_stat);
        MPI_Get_count(&proc_stat, MPI_INT, &p_count);
        if (p_count == 0) {
            // Process Request
            if (proc_num > 0) {
                // Processes available
                ret_proc = proc_stack[proc_num - 1];
                proc_num--;
                MPI_Isend(&ret_proc, 1, MPI_INT, proc_stat.MPI_SOURCE, PROCESS, MPI_COMM_WORLD, &proc_req);
            } else {
                // No processes available
                MPI_Isend(NULL, 0, MPI_INT, proc_stat.MPI_SOURCE, PROCESS, MPI_COMM_WORLD, &proc_req);
            }
        } else {
            // Process Return
            proc_stack[proc_num++] = ret_proc;
        }
    }

    // All processes back. Hull must be complete
    MPI_Isend(NULL, 0, MPI_INT, 0, HULL_POINT, MPI_COMM_WORLD, &proc_req);
    return NULL;
}
