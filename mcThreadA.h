//
// Created by Cristiano Rocha on 15/05/2024.
//

#ifndef SO_PROJECT_MCTHREADA_H
#define SO_PROJECT_MCTHREADA_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <stdbool.h>
#include "monteCarloAB.h"

#define BUFFER_SIZE 1024

int orientation(Point p, Point q, Point r);

bool onSegment(Point p, Point q, Point r);

bool doIntersect(Point p1, Point q1, Point p2, Point q2);

bool isInsidePolygon(Point polygon[], int n, Point p);

ssize_t read_line(int fd, char *buffer, size_t n);

int read_int(int fd);

void mcThreadA(int argc, char *argv[]);

#endif //SO_PROJECT_MCTHREADA_H
