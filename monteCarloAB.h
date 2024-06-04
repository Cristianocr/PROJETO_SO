//
// Created by Cristiano Rocha on 20/03/2024.
//

#ifndef SO_PROJECT_MONTECARLOAB_H
#define SO_PROJECT_MONTECARLOAB_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

typedef struct {
    double x;
    double y;
} Point;

int orientation(Point p, Point q, Point r);

bool onSegment(Point p, Point q, Point r);

bool doIntersect(Point p1, Point q1, Point p2, Point q2);

bool isInsidePolygon(Point polygon[], int n, Point p);

ssize_t read_line(int fd, char *buffer, size_t n);

int read_int(int fd);

void monteCarloAB(int argc, char *argv[]);

#endif //SO_PROJECT_MONTECARLOAB_H
