#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <float.h>
#include <mpi.h>
#include <pthread.h>
#include "utils.h"
#include "processPool.h"
#include <iostream>
#include <fstream>
#include <circular_linked_list.hpp>

MPI_Datatype MPI_POINT;

using namespace std;

vector<Point> serial_quick_hull(vector<Point>& points);
vector<Point> mergeHulls(vector<vector<Point>>& hulls, int low, int high);
vector<Point> merge(vector<Point>& left, vector<Point>& right);
void qh_helper(vector<Point>& points, circular_linked_list<Point>* hull, Point P, Point Q);

int main(int argc, char **argv) {
    int rank, size;
    struct Arguments arg;
    double time;
    vector<vector<Point>> pointsForProc;
    vector<MPI_Request> req;
    vector<MPI_Status> status;
    vector<Point> points;

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

    points.resize(arg.numPoints);
    loadFile(arg.inFile, points.data(), arg.numPoints);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    define_MPI_POINT(&MPI_POINT);

    req.resize(size);
    status.resize(size);
    pointsForProc.resize(size);


    if (rank == 0) {
        cout << "in rank 0" << endl;
        time = MPI_Wtime();
        Point* left = &points[0], *right = &points[0];
        int idx;
        double low, high;

        // Find the leftmost and rightmost points
        for (int i = 1; i < arg.numPoints; ++i) {
            if (points[i].x < left->x) {
                left = &points[i];
            }
            if (points[i].x > right->x) {
                right = &points[i];
            }
        }
        low = left->x;
        high = right->x;
        cout << "left: " << low  << endl;
        cout << "right: " << high << endl;

        for (int i = 1; i < arg.numPoints; ++i) {
            idx = (size * (points[i].x - low)) / (high - low);
            cout << "point: " << "(" << points[i].x << "," << points[i].y << ")" << endl;
            cout << "low: " << low << ", high: " << high << endl;
            cout << "proc: " << idx << endl;
            if (idx == size)
                idx--;
            pointsForProc[idx].push_back(points[i]);
        }
        for (int i = 1; i < size; ++i) {
            int count = pointsForProc[i].size();
            MPI_Send(&count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        for (int i = 1; i < size; ++i) {
            MPI_Send(pointsForProc[i].data(), pointsForProc[i].size(), MPI_POINT, i, 0, MPI_COMM_WORLD);
        }
    }
    else {
        int num;
        MPI_Recv(&num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status[rank]);
        vector<Point> my_points(num);

        MPI_Recv(my_points.data(), num, MPI_POINT, 0, 0, MPI_COMM_WORLD, &status[rank]);

        vector<Point> my_hull = serial_quick_hull(my_points);

        for (int i = 1; i < size; ++i) {
            int count = my_hull.size();
            MPI_Isend(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &req[rank]);
        }
        for (int i = 1; i < size; ++i) {
            MPI_Isend(my_hull.data(), my_hull.size(), MPI_POINT, 0, 0, MPI_COMM_WORLD, &req[rank]);
        }
    }

    if (rank == 0) {
        cout << "computing hull in rank 0" << endl;
        vector<vector<Point>> hulls(size);
        vector<int> hull_sizes(size);
        vector<Point> my_hull = serial_quick_hull(pointsForProc[0]);
        hulls[0] = my_hull;

        for (int i = 1; i < size; ++i) {
            MPI_Irecv(&hull_sizes[i], 1, MPI_INT, i,
                      0, MPI_COMM_WORLD, &req[i]);
        }

        for (int i = 1; i < size; ++i) {
            MPI_Wait(&req[i], &status[i]);
            hulls[i].resize(hull_sizes[i]);
        }

        for (int i = 1; i < size; ++i) {
            MPI_Irecv(hulls[i].data(), hulls[i].size(), MPI_POINT, i,
                      0, MPI_COMM_WORLD, &req[i]);
        }
        for (int i = 1; i < size; ++i) {
            MPI_Wait(&req[i], &status[i]);
        }
        auto final_hull = mergeHulls(hulls, 0, hulls.size() - 1);
        time = MPI_Wtime() - time;
        ofstream time_file, hull_file;
        time_file.open("partition_space_time_size_" + to_string(size));
        time_file << time << endl;
        time_file.close();
        hull_file.open(arg.outFile);
        hull_file << setprecision(6) << fixed;
        for (auto p : final_hull) {
            hull_file << p.x << ", " << p.y << endl;
        }
        hull_file.close();
    }
}

vector<Point> serial_quick_hull(vector<Point>& points) {
    Point *left, *right;
    vector<Point> top, bottom;
    circular_linked_list<Point>* hull;
    if (points.size() <= 3) {
        return points;
    }
    left = &points[0], right = &points[0];
    for (unsigned int i = 1; i < points.size(); ++i) {
        if (points[i].x < left->x) {
            left = &points[i];
        }
        if (points[i].x > right->x) {
            right = &points[i];
        }
    }
    for (unsigned int i = 0; i < points.size(); ++i) {
        int side = findSide(*left, *right, points[i]);
        if (side < 0) {
            bottom.push_back(points[i]);
        }
        else if (side > 0) {
            top.push_back(points[i]);
        }
    }
    hull = new circular_linked_list<Point>(*left);
    hull->insert_after(*right);

    qh_helper(top, hull, *left, *right);
    qh_helper(bottom, hull, *right, *left);

    return hull;
}

void qh_helper(vector<Point>& points, circular_linked_list<Point>* hull, Point P, Point Q) {
    if (points.size() == 0) {
        return;
    }
    double max_dist = -DBL_MAX;
    Point *farthest = nullptr;
    for (unsigned int i = 0; i < points.size(); ++i) {
        double dist = lineDist(P, Q, points[i]);
        if (dist > max_dist) {
            max_dist = dist;
            farthest = &points[i];
        }
    }
    hull.push_back(*farthest);
    vector<Point> left, right;
    for (unsigned int i = 0; i < points.size(); ++i) {
        Point p = points[i];
        if (findSide(P, *farthest, p) > 0) {
            left.push_back(p);
        }
        else if (findSide(*farthest, Q, p) > 0) {
            right.push_back(p);
        }
    }
    qh_helper(left, hull, P, *farthest);
    qh_helper(right, hull, *farthest, Q);
}

vector<Point> mergeHulls(vector<vector<Point>>& hulls, int low, int high) {
    if (high == low) {
        return hulls[high];
    }
    vector<Point> left = mergeHulls(hulls, low, (low + high) / 2);
    vector<Point> right = mergeHulls(hulls, (low + high) / 2 + 1, high);
    return merge(left, right);
}



