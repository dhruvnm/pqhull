#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "utils.h"

double randFrom(int min, int max) {
    return min + (rand() / (RAND_MAX / (double)(max - min)));
}

struct Point randDisk(int maxRadius) {
    struct Point p;
    double a, r;

    a = randFrom(0, 1) * 2 * M_PI;
    r = maxRadius * sqrt(randFrom(0, 1));

    p.x = r * cos(a);
    p.y = r * sin(a);

    return  p;
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

    points = (struct Point *) malloc(arg.numPoints * sizeof(struct Point));

    //Generate random points
    srand(time(NULL));
    for (i = 0; i < arg.numPoints; i++) {
        points[i] = randDisk(arg.radius);
    }

    //Save points to file
    writePointArrayToFile(arg.outFile, points, arg.numPoints);
}
