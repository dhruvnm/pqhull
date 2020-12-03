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

MPI_Datatype MPI_POINT;

using namespace std;

vector<Point> serial_quick_hull(vector<Point>& points);
vector<Point> mergeHulls(vector<vector<Point>*>& hulls, int low, int high);
vector<Point> merge(vector<Point>& left, vector<Point>& right);
void qh_helper(vector<Point>& points, vector<Point>* hull, Point P, Point Q);

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
        vector<vector<Point>*> hulls(size);
        vector<int> hull_sizes(size);
        vector<Point> my_hull = serial_quick_hull(pointsForProc[0]);
        hulls[0] = &my_hull;

        for (int i = 1; i < size; ++i) {
            MPI_Irecv(&hull_sizes[i], 1, MPI_INT, i,
                      0, MPI_COMM_WORLD, &req[i]);
        }
        for (int i = 1; i < size; ++i) {
            MPI_Wait(&req[i], &status[i]);
            hulls[i] = new vector<Point>(hull_sizes[i]);
        }
        for (int i = 1; i < size; ++i) {
            MPI_Irecv(hulls[i]->data(), hulls[i]->size(), MPI_POINT, i,
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
        for (auto p : final_hull) {
            hull_file << "(" << p.x << "," << p.y << ")" << endl;
        }
        hull_file.close();
    }
}

vector<Point> serial_quick_hull(vector<Point>& points) {
    Point *left, *right;
    vector<Point> top, bottom;
    vector<Point> hull;
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
            top.push_back(points[i]);
        }
        else if (side > 0) {
            bottom.push_back(points[i]);
        }
    }
    qh_helper(top, &hull, *left, *right);
    qh_helper(bottom, &hull, *right, *left);

    return hull;
}

void qh_helper(vector<Point>& points, vector<Point>* hull, Point P, Point Q) {
    if (points.size() == 0) {
        return;
    }
    double max_dist = -DBL_MAX;
    Point *farthest = NULL;
    for (unsigned int i = 0; i < points.size(); ++i) {
        double dist = lineDist(P, Q, points[i]);
        if (dist > max_dist) {
            max_dist = dist;
            farthest = &points[i];
        }
    }
    hull->push_back(*farthest);
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

vector<Point> mergeHulls(vector<vector<Point>*>& hulls, int low, int high) {
    if (high == low) {
        return *hulls[high];
    }
    vector<Point> left = mergeHulls(hulls, low, (low + high) / 2);
    vector<Point> right = mergeHulls(hulls, (low + high) / 2 + 1, high);
    return merge(left, right);
}

vector<Point> merge(vector<Point>& left, vector<Point>& right) {
    int n1 = left.size(), n2 = right.size();

    int ia = 0, ib = 0;
    for (int i=1; i<n1; i++)
        if (left[i].x > left[ia].x)
            ia = i;
    for (int i=1; i<n2; i++)
        if (right[i].x < right[ib].x)
            ib=i;

    int inda = ia, indb = ib;
    bool done = 0;
    while (!done)
    {
        done = 1;
        while (findSide(right[indb], left[inda], left[(inda+1)%n1]) >=0)
            inda = (inda + 1) % n1;

        while (findSide(left[inda], right[indb], right[(n2+indb-1)%n2]) <=0)
        {
            indb = (n2+indb-1)%n2;
            done = 0;
        }
    }

    int uppera = inda, upperb = indb;
    inda = ia, indb=ib;
    done = 0;
    while (!done)
    {
        done = 1;
        while (findSide(left[inda], right[indb], right[(indb+1)%n2])>=0)
            indb=(indb+1)%n2;

        while (findSide(right[indb], left[inda], left[(n1+inda-1)%n1])<=0)
        {
            inda=(n1+inda-1)%n1;
            done=0;
        }
    }

    int lowera = inda, lowerb = indb;
    vector<Point> ret;

    int ind = uppera;
    ret.push_back(left[uppera]);
    while (ind != lowera)
    {
        ind = (ind+1)%n1;
        ret.push_back(left[ind]);
    }

    ind = lowerb;
    ret.push_back(right[lowerb]);
    while (ind != upperb)
    {
        ind = (ind+1)%n2;
        ret.push_back(right[ind]);
    }
    return ret;
}

