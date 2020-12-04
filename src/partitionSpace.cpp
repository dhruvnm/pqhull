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
#include "circular_linked_list.hpp"

MPI_Datatype MPI_POINT;

using namespace std;

void output_vector(const string filename, vector<Point> v);
circular_linked_list<Point>* serial_quick_hull(vector<Point>& points);
circular_linked_list<Point>*  mergeHulls(vector<circular_linked_list<Point>*>& hulls,
                                         int low, int high);
circular_linked_list<Point>*  merge(circular_linked_list<Point>*, circular_linked_list<Point>*);
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

        for (int i = 1; i < arg.numPoints; ++i) {
            idx = (size * (points[i].x - low)) / (high - low);
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

        circular_linked_list<Point>* h = serial_quick_hull(my_points);
        auto my_hull = h->cll_to_vector();

        int count = my_hull.size();
        MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(my_hull.data(), my_hull.size(), MPI_POINT, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        vector<circular_linked_list<Point>*> hulls(size);
        vector<int> hull_sizes(size);
        vector<vector<Point>> vhulls(size);
        ofstream time_file, hull_file;

        hulls[0] = serial_quick_hull(pointsForProc[0]);

        for (int i = 1; i < size; ++i) {
            MPI_Irecv(&hull_sizes[i], 1, MPI_INT, i,
                      0, MPI_COMM_WORLD, &req[i]);
        }
        for (int i = 1; i < size; ++i) {
            MPI_Wait(&req[i], &status[i]);
            vhulls[i].resize(hull_sizes[i]);
        }

        for (int i = 1; i < size; ++i) {
            MPI_Irecv(vhulls[i].data(), vhulls[i].size(), MPI_POINT, i,
                      0, MPI_COMM_WORLD, &req[i]);
        }
        for (int i = 1; i < size; ++i) {
            MPI_Wait(&req[i], &status[i]);
            hulls[i] = circular_linked_list<Point>::vector_to_cll(vhulls[i]);
        }
        circular_linked_list<Point>* final_hull = mergeHulls(hulls, 0, hulls.size() - 1);
        time = MPI_Wtime() - time;

        vector<Point> fl = final_hull->cll_to_vector();
        time_file.open("partition_space_time_size_" + to_string(size));
        time_file << time << endl;
        output_vector(arg.outFile, fl);
    }
    MPI_Finalize();
    return 0;
}

void output_vector(const string filename, vector<Point> v) {
    ofstream hull_file;
    hull_file.open(filename);
    hull_file << setprecision(6) << fixed;
    for (auto p : v) {
        hull_file << p.x << ", " << p.y << endl;
    }
    hull_file.close();
}


circular_linked_list<Point>* serial_quick_hull(vector<Point>& points) {
    Point *left, *right;
    vector<Point> top, bottom;
    circular_linked_list<Point>* hull;
    if (points.size() <= 3) {
        return circular_linked_list<Point>::vector_to_cll(points);
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

    circular_linked_list<Point>* f = hull->next;

    qh_helper(top, hull, *left, *right);
    qh_helper(bottom, f, *right, *left);

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
    hull->insert_after(*farthest);
    circular_linked_list<Point>* f = hull->next;
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
    qh_helper(right, f, *farthest, Q);
}

circular_linked_list<Point>* mergeHulls(vector<circular_linked_list<Point>*>& hulls,
                                         int low, int high) {
    if (high == low) {
        return hulls[high];
    }
    circular_linked_list<Point>* left = mergeHulls(hulls, low, (low + high) / 2);
    circular_linked_list<Point>* right = mergeHulls(hulls, (low + high) / 2 + 1, high);
    return merge(left, right);
}

circular_linked_list<Point>* merge(circular_linked_list<Point>* left,
                                   circular_linked_list<Point>* right) {

    circular_linked_list<Point>* left_right_most = left, *temp1, *temp2, *temp3, *temp4;
    for (auto next = left->next; next->data.x > left_right_most->data.x; next = next->next) {
        left_right_most = left_right_most->next;
    }
    temp1 = left_right_most;
    temp2 = right;
    temp3 = left_right_most;
    temp4 = right;

    for (; ;) {
        if (findSide(temp1->data, temp2->data, temp1->prev->data) > 0) {
            temp1 = temp1->prev;
        } else if (findSide(temp1->data, temp2->data, temp2->next->data) > 0) {
            temp2 = temp2->next;
        } else {
            break;
        }
    }
    for (; ;) {
        if (findSide(temp3->data, temp4->data, temp3->next->data) < 0) {
            temp3 = temp3->next;
        } else if (findSide(temp3->data, temp4->data, temp4->prev->data) < 0) {
            temp4 = temp4->prev;
        } else {
            break;
        }
    }

    temp1->next = temp2;
    temp2->prev = temp1;
    temp4->next = temp3;
    temp3->prev = temp4;

    //left->assert_valid_cll();
    return left;
}


