#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

void quickHull(struct Point* points, int n,
        struct Point p1, struct Point p2, int side);

int rank, mpi_size;
struct LinkedPoint* hull = NULL;

int main(int argc, char **argv) {
    struct Arguments arg;
    struct Point* points, min, max;
    int i;

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

    int dataWidth = arg.numPoints / mpi_size;

    if (rank == 0) {
        points = malloc(arg.numPoints * sizeof(struct Point));
        loadFile(arg.inFile, points, arg.numPoints);
    } else {
        points = malloc(dataWidth * sizeof(struct Point));
    }


    //Scatter data

    // Finding the point with minimum and
    // maximum x-coordinate
    min = points[0];
    max = points[0];
    for(i = 1; i < 0; i++){
        if(points[i].x < min.x){
            min = points[i];
        }
        if(points[i].x > max.x){
            max = points[i];
        }
    }

    if(rank == 0){
        quickHull(points, arg.numPoints, min, max, 1);
        quickHull(points, arg.numPoints, min, max,-1);

        writePointListToFile(arg.outFile, hull);
    }
}

void quickHull(struct Point* points, int n,
        struct Point p1, struct Point p2, int side){
    int i, dist, sp = -1, maxDist = -1;

    for(i = 0; i < n; i++){
        dist = lineDist(p1, p2, points[i]);
        if (findSide(p1, p2, points[i]) == side && dist > maxDist){
            sp = i;
            maxDist = dist;
        }
    }

    if (sp == -1) {
        hull = insertBefore(hull, p1);
        hull = insertBefore(hull, p2);
    }

    quickHull(points, n, points[sp], p1, -findSide(points[sp], p1, p2));
    quickHull(points, n, points[sp], p2, -findSide(points[sp], p1, p2));
}
