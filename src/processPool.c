#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"

#include "utils.h"
#include "processPool.h"

MPI_Datatype MPI_LINKED_POINT;

int main(int argc, char **argv) {
    // Initialize MPI
    int rank, size, provided;
    //MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    //if (provided < MPI_THREAD_MULTIPLE) {
    //    printf("Thread support not given\n");
    //    exit(1);
    //}
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
        int i, t, b, side, *proc_stack, proc_num, total, top_proc, bot_proc, h_idxs[3], h_count;
        pthread_t thread_id;
        struct ProcManagerArgs args;
        MPI_Request dummy_req, hull_req;
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

        // Set up process manager
        //proc_num = size - 1;
        //total = size - 1;
        //proc_stack = (int *)malloc(total * sizeof(int));
        //for (i = 0; i < total; i++) {
        //    proc_stack[i] = i + 1;
        //}

        // Get two process from the stack
        top_proc = size - 1;
        bot_proc = size - 2;
        //proc_num -= 2;

        // Start process manager
        //args.proc_stack = proc_stack;
        //args.proc_num = proc_num;
        //args.total = total;
        //pthread_create(&thread_id, NULL, processManager, (void *)&args);

        // Call QuickHull
        MPI_Isend(&t, 1, MPI_INT, top_proc, BUF_SIZE, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);
        MPI_Isend(&b, 1, MPI_INT, bot_proc, BUF_SIZE, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);
        MPI_Isend(top, t+2, MPI_LINKED_POINT, top_proc, FUNC_CALL, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);
        MPI_Isend(bot, b+2, MPI_LINKED_POINT, bot_proc, FUNC_CALL, MPI_COMM_WORLD, &dummy_req);
        MPI_Request_free(&dummy_req);

        printf("Master sent initial data\n");

        free(top);
        free(bot);

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

        // Stop timing. Done computing hull 
        //pthread_join(thread_id, NULL);
        //free(proc_stack);

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
    //ProcManagerArgs *arguments = (ProcManagerArgs *)args;

    int *proc_stack, proc_num, ret_proc, p_count, i;
    MPI_Request proc_req;
    MPI_Status proc_stat;
    proc_stack = (int *)malloc(total * sizeof(int));
    proc_num = total - 2;
    //total = arguments->total;

    for (i = 0; i < proc_num; i++) {
        proc_stack[i] = i+2;
    }

    printf("Total: %d, Proc_num: %d\n", total, proc_num);

    while (proc_num != total) {
        printf("Waiting for requests\n");
        MPI_Irecv(&ret_proc, 1, MPI_INT, MPI_ANY_SOURCE, PROCESS, MPI_COMM_WORLD, &proc_req);
        MPI_Wait(&proc_req, &proc_stat);
        MPI_Get_count(&proc_stat, MPI_INT, &p_count);
        printf("Recieved a request\n");
        if (p_count == 0) {
            printf("Process request\n");
            // Process Request
            if (proc_num > 0) {
                // Processes available
                printf("Process available\n");
                ret_proc = proc_stack[proc_num - 1];
                proc_num--;
                MPI_Isend(&ret_proc, 1, MPI_INT, proc_stat.MPI_SOURCE, PROCESS, MPI_COMM_WORLD, &proc_req);
                MPI_Request_free(&proc_req);
            } else {
                // No processes available
                printf("No process available\n");
                MPI_Isend(NULL, 0, MPI_INT, proc_stat.MPI_SOURCE, PROCESS, MPI_COMM_WORLD, &proc_req);
                MPI_Request_free(&proc_req);
            }
        } else {
            // Process Return
            printf("Process return\n");
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
    printf("Rank %d: Entering Quickhull\n", rank);
    MPI_Request req;
    MPI_Status stat;
    struct LinkedPoint C, *PC, *CQ;
    double maxDist, dist;
    int b_count, i, retBuf[3], l, r, newProc, p_count;

    if (m == MESSAGE) {
        // Get points from a diff process
        // Get buffer size
        printf("Rank %d: Entered message initialization\n", rank);
        MPI_Irecv(&n, 1, MPI_INT, MPI_ANY_SOURCE, BUF_SIZE, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &stat);
        printf("Rank %d: Recieved buf size\n", rank);
        MPI_Get_count(&stat, MPI_INT, &b_count);
        if (b_count == 0) {
            // This is a signal for the proc to die
            return;
        }
        // Need two extra spots for P and Q
        points = (LinkedPoint *)malloc((n+2) * sizeof(struct LinkedPoint));
        printf("Rank %d: Malloc: %x\n", rank, points);

        // Get points
        MPI_Irecv(points, n+2, MPI_LINKED_POINT, MPI_ANY_SOURCE, FUNC_CALL, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &stat);

        printf("Rank %d: Recieved points\n", rank);

        P = points[n];
        Q = points[n+1];

        printf("Rank %d: Done recieving stuff\n", rank);
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

        printf("Rank %d: Found max dist point\n", rank);

        // Send the point idx back to master so it can be added to hull
        retBuf[0] = P.index;
        retBuf[1] = C.index;
        retBuf[2] = Q.index;

        MPI_Isend(&retBuf, 3, MPI_INT, 0, HULL_POINT, MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        printf("Rank %d: Sent point to master\n", rank);

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

        printf("Rank %d: Done splitting points\n", rank);

        // Free the points buffer
        free(points);

        printf("Rank %d: Before requesting a process\n", rank);

        // Request a new proc
        MPI_Isend(NULL, 0, MPI_INT, 1, PROCESS, MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);
        printf("Rank %d: Sent request for process\n", rank);
        MPI_Irecv(&newProc, 1, MPI_INT, 1, PROCESS, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &stat);
        MPI_Get_count(&stat, MPI_INT, &p_count);
        printf("Rank %d: Recieved a response\n", rank);

        if (p_count == 0) {
            // No process available. Make two serial recursive calls
            printf("Rank %d: Two Serial Recursive calls\n", rank);
            quickHull(PC, l, P, C, RECURSIVE, rank);
            quickHull(CQ, r, C, Q, RECURSIVE, rank);
        } else {
            // Process was available!
            printf("Rank %d: Process was available\n", rank);
            PC[l] = P;
            PC[l+1] = C;
            MPI_Isend(&l, 1, MPI_INT, newProc, BUF_SIZE, MPI_COMM_WORLD, &req);
            MPI_Request_free(&req);
            MPI_Isend(PC, l+2, MPI_LINKED_POINT, newProc, FUNC_CALL, MPI_COMM_WORLD, &req);
            MPI_Request_free(&req);
            quickHull(CQ, r, C, Q, RECURSIVE, rank);
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