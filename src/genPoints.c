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

struct Point randRect(int min, int max){
    struct Point p;

    p.x = randFrom(min, max);
    p.y = randFrom(min, max);

    return p;
}

struct Point randExp(double lambda) {
    struct Point p;
    double u = rand() / (RAND_MAX + 1.0);
    double v = rand() / (RAND_MAX + 1.0);
    p.x = -log(1-u) / lambda;
    p.y = -log(1-v) / lambda;
    return p;
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
        if (arg.radius > 0) {
            points[i] = randDisk(arg.radius);
        } else if (arg.min > 0 && arg.max > 0) {
            points[i] = randRect(arg.min, arg.max);
        } else if (arg.exp > 0) {}
            points[i] = randExp(arg.exp);
        }
    }

    // Exponential points
    // for (i = 0; i < arg.numPoints; i++) {
    //     points[i] = randExp(1.0);
    // }

    //Save points to file
    writePointArrayToFile(arg.outFile, points, arg.numPoints);
}
