#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "utils.h"

void loadFile(char* fileName, struct Point* point, int n){
    FILE *f;
    int i;

    f = fopen(fileName, "r");
    if (f == NULL) {
        printf("Error opening file: %s\n", fileName);
        exit(1);
    }

    for (i = 0; i < n; i++) {
        fscanf(f, "%lf, %lf\n", &point[i].x, &point[i].y);
    }

    fclose(f);
}

void writeFile(char* fileName, struct Point* point, int n){
    FILE *f;
    int i;

    f = fopen(fileName, "w");
    if (f == NULL) {
        printf("Error opening file: %s\n", fileName);
        exit(1);
    }

    for (i = 0; i < n; i++) {
        fprintf(f, "%lf, %lf\n", point[i].x, point[i].y);
    }

    fclose(f);
}

void parseArgs(int argc, char **argv, struct Arguments* arg){
    int opt, long_idx;

    static struct option long_opts[] = {
        {"infile",    required_argument, 0,  'i' },
        {"outfile",   required_argument, 0,  'o' },
        {"numpoints", required_argument, 0,  'n' },
        {"min",       required_argument, 0,  '-' },
        {"max",       required_argument, 0,  '+' },
        {"debug",     no_argument,       0,  'd' },
        {0, 0, 0, 0}
    };

    arg->inFile = NULL;
    arg->outFile = NULL;
    arg->numPoints = 0;
    arg->debug = 0;

    arg->min = -10;
    arg->max = 10;

    while ((opt = getopt_long(argc, argv, "i:o:n:d", long_opts, &long_idx)) != -1) {
        switch (opt) {
            case 'i' :
                arg->inFile = optarg;
                break;
            case 'o' :
                arg->outFile = optarg;
                break;
            case 'n' :
                arg->numPoints = atoi(optarg);
                break;
            case '-' :
                arg->min = atoi(optarg);
                break;
            case '+' :
                arg->max = atoi(optarg);
                break;
            case 'd' :
                arg->debug = 1;
                break;
            default:
                break;
        }
    }
}

/******************************************************************************/
void printPoint(struct Point p){
    printf("%10.3lf, %10.3lf\n", p.x, p.y);
}
