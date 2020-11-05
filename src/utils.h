#ifndef UTILS_H
#define UTILS_H

struct Arguments {
    //General arguments
    char* inFile;
    char* outFile;
    int numPoints;

    //For point generation
    int radius;
    int min;
    int max;

    int debug;
};

struct Point {
   double x;
   double y;
};

//Node of a circular doubly linked list
struct LinkedPoint {
   struct Point point;
   int index;
   struct LinkedPoint* prev;
   struct LinkedPoint* next;
};

/******************************************************************************/
// Returns the side of point p with respect to line (p1, p2)
int findSide(struct Point p1, struct Point p2, struct Point p);

// returns a value proportional to the distance between the point p
// and the line joining the points p1 and p2
int lineDist(struct Point p1, struct Point p2, struct Point p);

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

void parseArgs(int argc, char **argv, struct Arguments* arg);

/******************************************************************************/
void printPoint(struct Point p);

#endif