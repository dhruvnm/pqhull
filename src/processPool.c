#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"

#include "utils.h"
#include "processPool.h"

int main(int argc, char **argv) {
    // Initialize MPI
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Master Process
    if (rank == 0) {
        struct Arguments arg;
        struct Point* points;
        struct LinkedPoint *hull, *P, *Q, *top, *bot;
        int i, t, b;

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

        points = malloc(arg.numPoints * sizeof(struct Point));
        loadFile(arg.inFile, points, arg.numPoints);

        // Convert list of points to linked points
        hull = malloc(arg.numPoints * sizeof(struct LinkedPoint))
        for (i = 0; i < arg.numPoints; i++) {
            hull[i].point = points[i];
            hull[i].index = i;
            hull[i].prev = NULL;
            hull[i].next = NULL;
        }

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
        top = malloc(arg.numPoints * sizeof(struct LinkedPoint));
        bot = malloc(arg.numPoints * sizeof(struct LinkedPoint));
        t = 0;
        b = 0;
        for (i = 0; i < arg.numPoints; i++) {
            if (findSide(P->point, Q->point, hull[i].point) > 0) {
                top[t++] = hull[i];
            } else {
                bot[b++] = hull[i];
            }
        }


        writePointListToFile(arg.outFile, hull);
        return 0;
    }
}
