
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"

#include "utils.h"
#include "processPool.h"

MPI_Datatype MPI_LINKED_POINT;

int main(int argc, int **argv) {
    int rank, size;
    struct Arguments arg;
    struct Point* points;
    struct Point** PointForProc;
    struct int* numPoints;

    define_MPI_LINKED_POINT(&MPI_LINKED_POINT);

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
    points = (struct Point*) malloc(arg.numPoints * sizeof(struct Point));
    loadFile(arg.inFile, points, arg.numPoints);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        PointForProc = (struct Point**) malloc(size * sizeof(struct Point*));
        struct Point* left = &points[0], right = &points[0];
        int width, idx;

        for (int i = 0; i < size; ++i) {
            PointForProc[i] = calloc(arg.numPoints, sizeof(struct Point));
        }
        numPoints = (int *) calloc(size, sizeof(int));

        // Find the leftmost and rightmost points
        for (int i = 1; i < arg.numPoints; ++i) {
            if (points[i].x < left->x) {
                left = &points[i];
            }
            if (points[i].x > right.x) {
                right = &points[i];
            }
        }
        width = right->x - left->x;
        for (int i = 1; i < arg.numPoints; ++i) {
            idx = (points[i].x - low) / size;
            PointForProc[numPoints[idx]] = points[i];
            numPoints[idx]++;
        }
    }


}


