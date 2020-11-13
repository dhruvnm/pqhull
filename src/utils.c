#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "utils.h"
#include "mpi.h"

/******************************************************************************/
int findSide(struct Point p1, struct Point p2, struct Point p) {
    int v;

    v = (p.y - p1.y) * (p2.x - p1.x) - (p2.y - p1.y) * (p.x - p1.x);

    if (v < 0) {
        return -1;
    } else if (v == 0) {
        return 0;
    } else {
        return 1;
    }
}

int lineDist(struct Point p1, struct Point p2, struct Point p) {
    int v;

    v = ((p.y - p1.y) * (p2.x - p1.x) - (p2.y - p1.y) * (p.x - p1.x));

    if (v < 0) {
        return -1 * v;
    } else {
        return v;
    }
}

/******************************************************************************/
struct LinkedPoint* insertBefore(struct LinkedPoint* lp, struct Point p) {
    struct LinkedPoint* newNode;

    newNode = (struct LinkedPoint*) malloc(sizeof(struct LinkedPoint));
    newNode->point = p;
    if (lp == NULL) {
        //First element of list
        newNode->prev = newNode;
        newNode->next = newNode;
        return newNode;
    } else {
        //Second Element or later
        newNode->next = lp;
        newNode->prev = lp->prev;
        lp->prev = newNode;
        newNode->prev->next = newNode;
        return lp;
    }
}

struct LinkedPoint* insertAfter(struct LinkedPoint* lp, struct Point p) {
    struct LinkedPoint* newNode;

    newNode = (struct LinkedPoint*) malloc(sizeof(struct LinkedPoint));
    newNode->point = p;
    if (lp == NULL) {
        //First element of list
        newNode->prev = newNode;
        newNode->next = newNode;
        return newNode;
    } else {
        //Second Element or later
        newNode->next = lp->next;
        newNode->prev = lp;
        lp->next = newNode;
        newNode->next->prev = newNode;
        return lp;
    }
}

/******************************************************************************/
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

void writePointArrayToFile(char* fileName, struct Point* point, int n){
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

void writePointListToFile(char* fileName, struct LinkedPoint* head){
    FILE *f;
    struct LinkedPoint* lp;

    f = fopen(fileName, "w");
    if (f == NULL) {
        printf("Error opening file: %s\n", fileName);
        exit(1);
    }

    lp = head;
    do {
        fprintf(f, "%lf, %lf\n", lp->point.x, lp->point.y);
        lp=lp->next;
    } while (lp != head);

    fclose(f);
}

void parseArgs(int argc, char **argv, struct Arguments* arg){
    int opt, long_idx;

    static struct option long_opts[] = {
        {"infile",    required_argument, 0,  'i' },
        {"outfile",   required_argument, 0,  'o' },
        {"numpoints", required_argument, 0,  'n' },
        {"radius",    required_argument, 0,  'r' },
        {"min",       required_argument, 0,  '-' },
        {"max",       required_argument, 0,  '+' },
        {"debug",     no_argument,       0,  'd' },
        {0, 0, 0, 0}
    };

    arg->inFile = NULL;
    arg->outFile = NULL;
    arg->numPoints = 0;

    arg->radius = 10;
    arg->min = -10;
    arg->max = 10;

    arg->debug = 0;

    while ((opt = getopt_long(argc, argv, "i:o:n:r:d", long_opts, &long_idx)) != -1) {
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
            case 'r' :
                arg->radius = atoi(optarg);
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

void startTime(double* startTime) {
    *startTime = MPI_Wtime();
}

void endTime(int rank, int size, double startTime) {
    double delta, min, avg, max;

    delta = MPI_Wtime() - startTime;

    MPI_Reduce(&delta, &min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&delta, &avg, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    avg /= size;
    MPI_Reduce(&delta, &max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0){
        printf("TIME: Min: %.6f s Avg: %.6f s Max: %.6f s\n", min, avg, max);
    }
}
