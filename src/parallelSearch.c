#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"
#include "mpi.h"

void quickHull(struct Point* points, int n, struct Point p1, struct Point p2, int side);

// Custom MPI datatypes & functions
MPI_Datatype MPI_POINT;
MPI_Datatype MPI_POINT_DISTANCE;
MPI_Op MPI_POINT_X_MIN;
MPI_Op MPI_POINT_X_MAX;
MPI_Op MPI_POINT_DISTANCE_MAX;

// Process rank & size
int mpiRank, mpiSize;

// List of hull points
struct LinkedPoint* hull = NULL;

int main(int argc, char **argv) {
    struct Arguments arg;
    struct Point *allPoints, *points, min, max;
    int i, min_i, max_i, dataSize;
    double time;

    // Parse Arguments
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

    // Initalize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    define_MPI_POINT(&MPI_POINT);
    define_MPI_POINT_DISTANCE(&MPI_POINT_DISTANCE);

    MPI_Op_create((MPI_User_function *) define_MPI_POINT_X_MIN, 1, &MPI_POINT_X_MIN);
    MPI_Op_create((MPI_User_function *) define_MPI_POINT_X_MAX, 1, &MPI_POINT_X_MAX);
    MPI_Op_create((MPI_User_function *) define_MPI_POINT_DISTANCE_MAX, 1, &MPI_POINT_DISTANCE_MAX);

    // Load Data
    if (mpiRank == 0) {
        allPoints = (struct Point*) malloc(arg.numPoints * sizeof(struct Point));
        loadFile(arg.inFile, allPoints, arg.numPoints);
    }
    dataSize = arg.numPoints / mpiSize;
    points = (struct Point*) malloc(dataSize * sizeof(struct Point));

    //Being Logic
    startTime(&time);

    //Scatter data
    MPI_Scatter(allPoints, dataSize, MPI_POINT, points, dataSize, MPI_POINT, 0, MPI_COMM_WORLD);

    // Finding the point with minimum and maximum x-coordinate
    min_i = 0;
    max_i = 0;
    for (i = 1; i < dataSize; i++) {
        if(points[i].x < points[min_i].x){
            min_i = i;
        }
        if(points[i].x > points[max_i].x){
            max_i = i;
        }
    }
    MPI_Allreduce(&points[min_i], &min, 1, MPI_POINT, MPI_POINT_X_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&points[max_i], &max, 1, MPI_POINT, MPI_POINT_X_MAX, MPI_COMM_WORLD);

    quickHull(points, dataSize, min, max, 1);
    quickHull(points, dataSize, min, max,-1);

    printf("END\n");

    //End Time
    endTime(mpiRank, mpiSize, time);
    MPI_Finalize();

    if (mpiRank == 0) {
        writePointListToFile(arg.outFile, hull);
    }
}

void quickHull(struct Point* points, int n, struct Point p1, struct Point p2, int side){
    struct PointDistance pd, newPD;
    int i, dist, sp = -1, maxDist = -1;

    //Scatter search parameters
    //printf("CHECK 10\n");
    for(i = 0; i < n; i++){
        dist = lineDist(p1, p2, points[i]);
        if (findSide(p1, p2, points[i]) == side && dist > maxDist){
            sp = i;
            maxDist = dist;
        }
    }

    //printf("CHECK 20\n");
    if (sp == -1) {
        pd.valid = 0;
    } else {
        pd.valid = 1;
        pd.point = points[sp];
        pd.dist = maxDist;
    }

    //printf("CHECK 30\n");
    MPI_Allreduce(&pd, &newPD, 1, MPI_POINT_DISTANCE, MPI_POINT_DISTANCE_MAX, MPI_COMM_WORLD);

    //printf("CHECK 40\n");
    if (!newPD.valid) {
        hull = insertBefore(hull, p1);
        hull = insertBefore(hull, p2);
    }
    //printf("CHECK 50\n");

    quickHull(points, n, points[sp], p1, -findSide(newPD.point, p1, p2));
    quickHull(points, n, points[sp], p2, -findSide(newPD.point, p1, p2));
}
