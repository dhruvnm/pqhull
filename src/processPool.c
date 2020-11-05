#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

int main(int argc, char **argv) {
    struct Arguments arg;
    struct Point* points;
    struct LinkedPoint* hull;

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

    hull = NULL;

    //TODO

    writePointListToFile(arg.outFile, hull);
}
