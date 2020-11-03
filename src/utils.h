struct Arguments {
    char* inFile;
    char* outFile;
    int numPoints;

    int min;
    int max;

    int debug;
};

struct Point {
   double x;
   double y;
};

/******************************************************************************/
void loadFile(char* fileName, struct Point* point, int n);

void writeFile(char* fileName, struct Point* point, int n);

void parseArgs(int argc, char **argv, struct Arguments* arg);

/******************************************************************************/
void printPoint(struct Point p);
