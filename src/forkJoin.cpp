#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "mpi.h"
#include "utils.h"

MPI_Datatype MPI_POINT;

struct PointCloud {
    std::vector<Point> points;

    std::vector<Point> quickHull();

    void work();

   private:
    std::vector<Point> delegateQP(const std::vector<int> &indices, Point P, Point C, Point Q, int calls);
    std::vector<Point> qpHelper(const std::vector<int> &indices, Point P, Point Q, int calls);

    std::vector<Point> spliceHulls(const std::vector<Point> &hullA, const std::vector<Point> &hullB);
    void cancel(int calls);
};

int main(int argc, char **argv) {
    // Initialize MPI
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct Arguments arg;

    define_MPI_POINT(&MPI_POINT);

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

    PointCloud worker = PointCloud{};

    worker.points.resize(arg.numPoints);

    loadFile(arg.inFile, &worker.points[0], arg.numPoints);

    std::vector<Point> hull;
    double time;
    startTime(&time);
    if (rank == 0) {
        // Master Process
        hull = worker.quickHull();
    } else {
        worker.work();
    }
    endTime(rank, size, time);

    if (rank == 0) {
        writePointArrayToFile(arg.outFile, &hull[0], hull.size());
    }

    MPI_Finalize();
    return 0;
}

std::vector<Point> PointCloud::quickHull() {
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int calls = log2(size);
    if (this->points.size() <= 3) {
        cancel(calls);
        return this->points;
    }

    struct Point L = {
        INFINITY,
        0,
    };

    struct Point R = {
        -INFINITY,
        0,
    };

    std::vector<int> indices;
    for (unsigned int i = 0; i < this->points.size(); i++) {
        L = *extremaPoint(&L, &points[i], -1);
        R = *extremaPoint(&R, &points[i], 1);

        indices.push_back(i);
    }

    return this->delegateQP(indices, L, R, L, calls);
}

void PointCloud::work() {
    int parent;
    MPI_Recv(&parent, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int calls;
    MPI_Recv(&calls, 1, MPI_POINT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    calls--;

    int len;
    MPI_Recv(&len, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (len==-1) {
        cancel(calls);
        return;
    }

    std::vector<int> indices;
    indices.resize(len);
    MPI_Recv(&indices[0], len, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    struct Point P, Q;
    MPI_Recv(&P, 1, MPI_POINT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&Q, 1, MPI_POINT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);



    auto hull = this->qpHelper(indices, P, Q, calls);

    len = hull.size();
    MPI_Send(&len, 1, MPI_INT, parent, 0, MPI_COMM_WORLD);
    MPI_Send(&hull[0], len, MPI_POINT, parent, 0, MPI_COMM_WORLD);
}

void PointCloud::cancel(int calls) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (calls <= 0) {
        return;
    }


    int dest = rank + (1 << (calls-1));
    MPI_Send(&rank, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);  // Parent

    MPI_Send(&calls, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

    int size = -1;
    MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
}

std::vector<Point> PointCloud::delegateQP(const std::vector<int> &indices, Point P, Point C, Point Q, int calls) {
    std::vector<int> A_indices;
    std::vector<int> B_indices;

    for (auto i : indices) {
        if (findSide(P, C, this->points[i]) > 0) {
            A_indices.push_back(i);
        }
        if (findSide(C, Q, this->points[i]) > 0) {
            B_indices.push_back(i);
        }
    }

    std::vector<Point> A_hull, B_hull;

    if (calls <= 0) {
        A_hull = this->qpHelper(A_indices, P, C, calls - 1);
        B_hull = this->qpHelper(B_indices, C, Q, calls - 1);
    } else {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        int dest = rank + (1 << (calls-1));

        MPI_Send(&rank, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);  // Parent

        MPI_Send(&calls, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

        int size = B_indices.size();
        MPI_Send(&size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

        MPI_Send(&B_indices[0], size, MPI_INT, dest, 0, MPI_COMM_WORLD);

        MPI_Send(&C, 1, MPI_POINT, dest, 0, MPI_COMM_WORLD);
        MPI_Send(&Q, 1, MPI_POINT, dest, 0, MPI_COMM_WORLD);

        A_hull = this->qpHelper(A_indices, P, C, calls - 1);

        MPI_Recv(&size, 1, MPI_INT, dest, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        B_hull.resize(size);
        MPI_Recv(&B_hull[0], size, MPI_POINT, dest, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    return this->spliceHulls(B_hull, A_hull);
}

std::vector<Point> PointCloud::spliceHulls(const std::vector<Point> &hullA, const std::vector<Point> &hullB) {
    std::vector<Point> hull = hullA;

    for (auto point : hullB) {
        if (point != hullA[0] && point != hullA[hullA.size() - 1]) {
            hull.push_back(point);
        }
    }

    return hull;
}

std::vector<Point> PointCloud::qpHelper(const std::vector<int> &indices, Point P, Point Q, int calls) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (indices.size() == 0) {
        cancel(calls);
        std::vector<Point> hull;
        hull.push_back(Q);
        hull.push_back(P);
        return hull;
    }

    double max_dist = -INFINITY;
    struct Point C;
    for (auto i : indices) {
        double dist = lineDist(P, Q, this->points[i]);
        if (dist > max_dist) {
            max_dist = dist;
            C = this->points[i];
        }
    }

    return this->delegateQP(indices, P, C, Q, calls);
}
