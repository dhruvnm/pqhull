#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

double randfrom(int min, int max) {
    return min + (rand() / (RAND_MAX / (double)(max - min)));
}

int main(int argc, char **argv) {
    struct Arguments arg;
    struct Point* points;

    int i;

    parseArgs(argc, argv, &arg);
    if (arg.numPoints <= 0) {
        printf("NumPoints %d must be larger than 0\n", arg.numPoints);
        exit(1);
    } else if (arg.outFile == NULL) {
        printf("Outfile is undefined\n");
        exit(1);
    }

    points = malloc(arg.numPoints * sizeof(struct Point));

    //Generate random points
    srand(time(NULL));
    for (i = 0; i < arg.numPoints; i++) {
        points[i].x = randfrom(arg.min, arg.max);
        points[i].y = randfrom(arg.min, arg.max);
    }

    //Save points to file
    writeFile(arg.outFile, points, arg.numPoints);

    if (arg.debug) {
        struct Point* points_reloaded;
        points_reloaded = malloc(arg.numPoints * sizeof(struct Point));
        loadFile(arg.outFile, points_reloaded, arg.numPoints);

        for (i = 0; i < arg.numPoints; i++) {
            printPoint(points[i]);
            printPoint(points_reloaded[i]);
            printf("\n");
        }

    }
}
