#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "utils.h"
#include "mpi.h"


/******************************************************************************/
void define_MPI_POINT(MPI_Datatype* MPI_POINT){
    //Collect necessary data for MPI
    int numFields = 2;
    int fieldLengths[numFields] = {1,1};
    MPI_Aint fieldOffsets[numFields];
    MPI_Datatype fieldTypes[numFields] = {MPI_DOUBLE, MPI_DOUBLE};

    //Fill field offsets with correct values
    fieldOffsets[0] = offsetof(struct Point, x);
    fieldOffsets[1] = offsetof(struct Point, y);

    //Submit info to MPI
    MPI_Type_create_struct(numFields, fieldLengths, fieldOffsets, fieldTypes, MPI_POINT);
    MPI_Type_commit(MPI_POINT);
}

void define_MPI_POINT_X_MIN(struct Point *in, struct Point *out, int *len, MPI_Datatype *typeptr){
    struct Point *pa, *pb;
    int i = 0;

    pa = (struct Point*) in;
    pb = (struct Point*) out;

    if (*len != 1){
        printf("Warning: this function may not work for lengths != 1\n");
    }

    if(&(pa[i]) == extremaPoint(&(pa[i]), &(pb[i]), -1)){
        pb[i] = pa[i];
    }
}

void define_MPI_POINT_X_MAX(struct Point *in, struct Point *out, int *len, MPI_Datatype *typeptr){
    struct Point *pa, *pb;
    int i = 0;

    pa = (struct Point*) in;
    pb = (struct Point*) out;

    if (*len != 1){
        printf("Warning: this function may not work for lengths != 1\n");
    }

    if(&(pa[i]) == extremaPoint(&(pa[i]), &(pb[i]), 1)){
        pb[i] = pa[i];
    }
}

/******************************************************************************/
void define_MPI_LINKED_POINT(MPI_Datatype* MPI_LINKED_POINT){
    //Collect necessary data for MPI
    int numFields = 5;
    int fieldLengths[numFields] = {1,1,1,1,1};
    MPI_Aint fieldOffsets[numFields];
    MPI_Datatype fieldTypes[numFields] = {MPI_DOUBLE, MPI_DOUBLE, MPI_LONG_LONG, MPI_LONG_LONG, MPI_INT};

    //Fill field offsets with correct values
    fieldOffsets[0] = offsetof(struct LinkedPoint, point) + offsetof(struct Point, x);
    fieldOffsets[1] = offsetof(struct LinkedPoint, point) + offsetof(struct Point, y);
    fieldOffsets[2] = offsetof(struct LinkedPoint, prev);
    fieldOffsets[3] = offsetof(struct LinkedPoint, next);
    fieldOffsets[4] = offsetof(struct LinkedPoint, index);

    //Submit info to MPI
    MPI_Type_create_struct(numFields, fieldLengths, fieldOffsets, fieldTypes, MPI_LINKED_POINT);
    MPI_Type_commit(MPI_LINKED_POINT);
}

/******************************************************************************/
void define_MPI_POINT_DISTANCE(MPI_Datatype* MPI_POINT_DISTANCE){
    //Collect necessary data for MPI
    int numFields = 4;
    int fieldLengths[numFields] = {1,1,1,1};
    MPI_Aint fieldOffsets[numFields];
    MPI_Datatype fieldTypes[numFields] = {MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_INT};

    //Fill field offsets with correct values
    fieldOffsets[0] = offsetof(struct PointDistance, point) + offsetof(struct Point, x);
    fieldOffsets[1] = offsetof(struct PointDistance, point) + offsetof(struct Point, y);
    fieldOffsets[2] = offsetof(struct PointDistance, valid);
    fieldOffsets[3] = offsetof(struct PointDistance, dist);

    //Submit info to MPI
    MPI_Type_create_struct(numFields, fieldLengths, fieldOffsets, fieldTypes, MPI_POINT_DISTANCE);
    MPI_Type_commit(MPI_POINT_DISTANCE);
}

void define_MPI_POINT_DISTANCE_MAX(struct PointDistance *in, struct PointDistance *out, int *len, MPI_Datatype *typeptr){
    struct PointDistance *pda, *pdb;
    int i = 0;

    pda = (struct PointDistance*) in;
    pdb = (struct PointDistance*) out;

    if (*len != 1){
        printf("Warning: this function may not work for lengths != 1\n");
    }
    printf("CHECK 10\n");
    if(pdb[i].valid == 0){
        pdb[i] = pda[i];
    } else if (pda[i].valid && pdb[i].valid) {
        if (pda[i].dist > pdb[i].dist) {
            pdb[i] = pda[i];
        } else if (pda[i].dist < pdb[i].dist) {
            pdb[i] = pdb[i];
        } else {
            if(&(pdb[i].point) == extremaPoint(&(pda[i].point), &(pdb[i].point), -1)){
                pdb[i] = pdb[i];
            } else {
                pdb[i] = pda[i];
            }
        }
    }
    printf("CHECK 20\n");
}

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

struct Point* extremaPoint(struct Point* p1, struct Point* p2, int d) {
    double xDelta, yDelta;

    xDelta = (p1->x - p2->x) * (double) d;
    yDelta = (p1->y - p2->y) * (double) d;

    if (xDelta == 0) {
        if (yDelta > 0) {
            return p1;
        } else {
            return p2;
        }
    } else if (xDelta > 0) {
        return p1;
    } else {
        return p2;
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
