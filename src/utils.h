#ifndef UTILS_H
#define UTILS_H

#include "mpi.h"

struct Arguments {
    // General arguments
    char* inFile;
    char* outFile;
    int numPoints;

    int debug;

    // For point generation
    int radius;
    int min;
    int max;

    int serial; // Should the program be run without MPI
};

struct Point {
    double x;
    double y;

    bool operator==(const Point& rhs) const { return this->x == rhs.x && this->y == rhs.y; }

    bool operator!=(const Point& rhs) const { return !(*this == rhs); }
};

// Node of a circular doubly linked list
struct LinkedPoint {
   struct Point point;
   struct LinkedPoint* prev;
   struct LinkedPoint* next;
   int index;
};

// Used in ParallelSearch to reduce over points in which some are not valid
struct PointDistance {
    struct Point point;
    double dist;
    int valid;
};

/******************************************************************************/
void define_MPI_POINT(MPI_Datatype* MPI_POINT);
void define_MPI_POINT_X_MIN(struct Point *in, struct Point *out, int *len, MPI_Datatype *typeptr);
void define_MPI_POINT_X_MAX(struct Point *in, struct Point *out, int *len, MPI_Datatype *typeptr);

/******************************************************************************/
void define_MPI_LINKED_POINT(MPI_Datatype* MPI_LINKED_POINT);

/******************************************************************************/
void define_MPI_POINT_DISTANCE(MPI_Datatype* MPI_POINT_DISTANCE);
void define_MPI_POINT_DISTANCE_MAX(struct PointDistance *in, struct PointDistance *out, int *len, MPI_Datatype *typeptr);

/******************************************************************************/
// Returns the side of point p with respect to line (p1, p2)
int findSide(struct Point p1, struct Point p2, struct Point p);

// returns a value proportional to the distance between the point p
// and the line joining the points p1 and p2
double lineDist(struct Point p1, struct Point p2, struct Point p);

// Get point either left/bottom most (-1) or right/top most (1)
struct Point* extremaPoint(struct Point* p1, struct Point* p2, int d);

/******************************************************************************/
// Insert point p into the linked list before lp
// If lp is null create a new linked list
// Because lp is circular, inserting before the head is equivalent to adding to the end
// Return lp
struct LinkedPoint* insertBefore(struct LinkedPoint* lp, struct Point p);

// Insert point p into the linked list after lp
// If lp is null create a new linked list
// Return lp
struct LinkedPoint* insertAfter(struct LinkedPoint* lp, struct Point p);

/******************************************************************************/
void loadFile(char* fileName, struct Point* point, int n);

void writePointArrayToFile(char* fileName, struct Point* point, int n);

void writePointListToFile(char* fileName, struct LinkedPoint* point);

void writePointListToFileReverse(char* fileName, struct LinkedPoint* point);

void parseArgs(int argc, char **argv, struct Arguments* arg);

/******************************************************************************/
void printPoint(struct Point p);

// Standard function to start MPI timing
void startTime(double* startTime);

// Standard function to print MPI timing
void endTime(int rank, int size, double startTime);

// Standard function to time when not using MPI
void endTimeSingle(double startTime);

#endif
