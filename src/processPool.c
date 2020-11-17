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
        if (size < 4) {
            printf("This program requires at least 4 processes\n");
            exit(1);
        }

        struct Arguments arg;
        struct Point* points;
        struct LinkedPoint *hull, *P, *Q, *top, *bot;
        int i, t, b, side, top_proc, bot_proc, h_idxs[3], h_count;
        MPI_Request dummy_req, hull_req;
        MPI_Status hull_stat;
        double start;

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

        startTime(&start);

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
            side = findSide(P->point, Q->point, hull[i].point);
            if ( side > 0) {
                top[t++] = hull[i];
            } else if (side < 0) {
                bot[b++] = hull[i];
            }
        }

        // Add in the two end points to the end of the buffer
        top[t] = *P;
        top[t+1] = *Q;
        bot[b] = *Q;
        bot[b+1] = *P;

        // Claim two processes
        top_proc = size - 1;
        bot_proc = size - 2;

        // Call QuickHull
        MPI_Isend(&t, 1, MPI_INT, top_proc, BUF_SIZE, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);
        MPI_Isend(&b, 1, MPI_INT, bot_proc, BUF_SIZE, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);
        MPI_Isend(top, t+2, MPI_LINKED_POINT, top_proc, FUNC_CALL, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);
        MPI_Isend(bot, b+2, MPI_LINKED_POINT, bot_proc, FUNC_CALL, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);

        while (true) {
            MPI_Irecv(h_idxs, 3, MPI_INT, MPI_ANY_SOURCE, HULL_POINT, MPI_COMM_WORLD, &hull_req);
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

        free(top);
        free(bot);

        endTimeSingle(start);

        writePointListToFileReverse(arg.outFile, P);
        free(hull);
    } else if (rank == 1) {
        processManager(size - 2);
    } else {
        // Worker processes
        LinkedPoint P; // dummy point to pass as an arg
        quickHull(NULL, 0, P, P, MESSAGE, rank);
    }

    // Done executing parallel code. 
    MPI_Finalize();
    return 0;
}

void processManager(int total) {
    int *proc_stack, proc_num, ret_proc, p_count, i;
    MPI_Request proc_req;
    MPI_Status proc_stat;
    proc_stack = (int *)malloc(total * sizeof(int));
    proc_num = total - 2;

    for (i = 0; i < proc_num; i++) {
        proc_stack[i] = i+2;
    }

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
                MPI_Request_free(&proc_req);
            } else {
                // No processes available
                MPI_Isend(NULL, 0, MPI_INT, proc_stat.MPI_SOURCE, PROCESS, MPI_COMM_WORLD, &proc_req);
                MPI_Request_free(&proc_req);
            }
        } else {
            // Process Return
            proc_stack[proc_num++] = ret_proc;
        }
    }

    // All processes back. Hull must be complete
    MPI_Isend(NULL, 0, MPI_INT, 0, HULL_POINT, MPI_COMM_WORLD, &proc_req);
    MPI_Request_free(&proc_req);

    // Tell all other processes to die
    for (i = 0; i < total; i++) {
        MPI_Isend(NULL, 0, MPI_INT, proc_stack[i], BUF_SIZE, MPI_COMM_WORLD, &proc_req);
        MPI_Request_free(&proc_req);
    }
}

void quickHull(LinkedPoint *points, int n, LinkedPoint P, LinkedPoint Q, Mode m, int rank) {
    MPI_Request req;
    MPI_Status stat;
    struct LinkedPoint C, *PC, *CQ;
    double maxDist, dist;
    int b_count, i, retBuf[3], l, r, newProc, p_count;

    if (m == MESSAGE) {
        // Get points from a diff process
        // Get buffer size
        MPI_Irecv(&n, 1, MPI_INT, MPI_ANY_SOURCE, BUF_SIZE, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &stat);
        MPI_Get_count(&stat, MPI_INT, &b_count);
        if (b_count == 0) {
            // This is a signal for the proc to die
            return;
        }
        // Need two extra spots for P and Q
        points = (LinkedPoint *)malloc((n+2) * sizeof(struct LinkedPoint));

        // Get points
        MPI_Irecv(points, n+2, MPI_LINKED_POINT, MPI_ANY_SOURCE, FUNC_CALL, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &stat);

        P = points[n];
        Q = points[n+1];
    }

    if (n != 0)
    {
        // Find max distance point from line PQ
        C = points[0];
        maxDist = lineDist(P.point, Q.point, C.point);
        for (i = 1; i < n; i++) {
            dist = lineDist(P.point, Q.point, points[i].point);
            if (dist > maxDist) {
                maxDist = dist;
                C = points[i];
            }
        }

        // Send the point idx back to master so it can be added to hull
        retBuf[0] = P.index;
        retBuf[1] = C.index;
        retBuf[2] = Q.index;

        MPI_Isend(retBuf, 3, MPI_INT, 0, HULL_POINT, MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        // Split pointsets
        PC = (LinkedPoint *)malloc((n+2) * sizeof(struct LinkedPoint));
        CQ = (LinkedPoint *)malloc((n+2) * sizeof(struct LinkedPoint));
        l = 0;
        r = 0;
        for (i = 0; i < n; i++) {
            if (findSide(P.point, C.point, points[i].point) > 0) {
                PC[l++] = points[i];
            }
            else if (findSide(C.point, Q.point, points[i].point) > 0) {
                CQ[r++] = points[i];
            }
        }

        // Free the points buffer
        free(points);

        // Request a new proc
        MPI_Isend(NULL, 0, MPI_INT, 1, PROCESS, MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);
        MPI_Irecv(&newProc, 1, MPI_INT, 1, PROCESS, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &stat);
        MPI_Get_count(&stat, MPI_INT, &p_count);

        if (p_count == 0) {
            // No process available. Make two serial recursive calls
            quickHull(PC, l, P, C, RECURSIVE, rank);
            quickHull(CQ, r, C, Q, RECURSIVE, rank);
        } else {
            // Process was available!
            PC[l] = P;
            PC[l+1] = C;
            MPI_Isend(&l, 1, MPI_INT, newProc, BUF_SIZE, MPI_COMM_WORLD, &req);
            MPI_Request_free(&req);
            MPI_Isend(PC, l+2, MPI_LINKED_POINT, newProc, FUNC_CALL, MPI_COMM_WORLD, &req);
            quickHull(CQ, r, C, Q, RECURSIVE, rank);
            MPI_Wait(&req, &stat);
            free(PC);
        }
    } else {
        free(points);
    }

    // Return process if not called recursively
    if (m == MESSAGE) {
        // Send proc back
        MPI_Isend(&rank, 1, MPI_INT, 1, PROCESS, MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);
        // Call itself in message mode
        quickHull(NULL, 0, P, P, MESSAGE, rank);
    }
}