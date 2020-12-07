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

struct Arguments arg;
struct LinkedPoint* hull = NULL;
int mpiRank, mpiSize;

int main(int argc, char **argv) {
    struct Point *allPoints, *points, *side1, *side2, min, max;
    int i, min_i, max_i, dataSize, side1_i, side2_i;
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
    if(arg.serial){
        mpiRank = 0;
    } else {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
        MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

        define_MPI_POINT(&MPI_POINT);
        define_MPI_POINT_DISTANCE(&MPI_POINT_DISTANCE);

        MPI_Op_create((MPI_User_function *) define_MPI_POINT_X_MIN, 1, &MPI_POINT_X_MIN);
        MPI_Op_create((MPI_User_function *) define_MPI_POINT_X_MAX, 1, &MPI_POINT_X_MAX);
        MPI_Op_create((MPI_User_function *) define_MPI_POINT_DISTANCE_MAX, 1, &MPI_POINT_DISTANCE_MAX);
    }

    // Load Data
    if (mpiRank == 0) {
        allPoints = (struct Point*) malloc(arg.numPoints * sizeof(struct Point));
        loadFile(arg.inFile, allPoints, arg.numPoints);
    } else {
        allPoints = NULL;
    }

    if(arg.serial){
        dataSize = arg.numPoints;
        points = allPoints;
        points = allPoints;
    } else {
        dataSize = arg.numPoints / mpiSize;
        points = (struct Point*) malloc(dataSize * sizeof(struct Point));
    }
    side1 = (struct Point*) malloc(dataSize * sizeof(struct Point));
    side2 = (struct Point*) malloc(dataSize * sizeof(struct Point));

    //Being Logic
    startTime(&time);

    //Scatter data
    if(!arg.serial){
        MPI_Scatter(allPoints, dataSize, MPI_POINT, points, dataSize, MPI_POINT, 0, MPI_COMM_WORLD);
    }

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

    if(arg.serial){
        min = points[min_i];
        max = points[max_i];
    } else {
        MPI_Allreduce(&points[min_i], &min, 1, MPI_POINT, MPI_POINT_X_MIN, MPI_COMM_WORLD);
        MPI_Allreduce(&points[max_i], &max, 1, MPI_POINT, MPI_POINT_X_MAX, MPI_COMM_WORLD);
    }

    side1_i = 0;
    side2_i = 0;
    for (i = 0; i < dataSize; i++) {
        if(findSide(min, max, points[i]) == 1){
            side1[side1_i] = points[i];
            side1_i++;
        } else {
            side2[side2_i] = points[i];
            side2_i++;
        }
    }
    free(points);

    quickHull(side1, side1_i, min, max, 1);
    quickHull(side2, side2_i, max, min, 1);

    //End Time
    if(arg.serial){
        endTimeSingle(time);
    } else {
        endTime(mpiRank, mpiSize, time);
        MPI_Finalize();
    }

    if (mpiRank == 0) {
        writePointListToFileReverse(arg.outFile, hull);
    }
}

void quickHull(struct Point* points, int n, struct Point p1, struct Point p2, int side){
    struct PointDistance pd, newPD;
    struct Point *side1, *side2;
    int i, sp, side1_i, side2_i;
    double dist, maxDist = -1;

    sp = -1;
    for(i = 0; i < n; i++){
        dist = lineDist(p1, p2, points[i]);
        if (findSide(p1, p2, points[i]) == side && dist > maxDist){
            sp = i;
            maxDist = dist;
        }
    }

    if (sp < 0) {
        pd.valid = 0;
        pd.dist = 0;
    } else {
        pd.valid = 1;
        pd.point = points[sp];
        pd.dist = maxDist;
    }

    if(arg.serial) {
        newPD = pd;
    } else {
        MPI_Allreduce(&pd, &newPD, 1, MPI_POINT_DISTANCE, MPI_POINT_DISTANCE_MAX, MPI_COMM_WORLD);
    }

    if (!newPD.valid) {
        if (mpiRank == 0) {
            hull = insertBefore(hull, p1);
        }
    } else {
        side1 = (struct Point*) malloc(n * sizeof(struct Point));
        side2 = (struct Point*) malloc(n * sizeof(struct Point));
        side1_i = 0;
        side2_i = 0;
        for (i = 0; i < n; i++) {
            if(findSide(p1, newPD.point, points[i]) == 1){
                side1[side1_i] = points[i];
                side1_i++;
            } else if(findSide(newPD.point, p2, points[i]) == 1){
                side2[side2_i] = points[i];
                side2_i++;
            }
        }
        free(points);
        quickHull(side1, side1_i, p1, newPD.point, findSide(newPD.point, p1, p2));
        quickHull(side2, side2_i, newPD.point, p2, findSide(newPD.point, p1, p2));
    }
}
