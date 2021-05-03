#ifndef _MWM_H
#define _MWM_H
#include <stdio.h>
#include <memory.h> // for memset
#include <stdlib.h> // for malloc, free
#include "parameters.h"
//#include "sort_deduplication.h" // for maximum_element

#define matrix(row, col) matrix[NUM_PORT * row + col]

int pole_input[NUM_PORT], pole_output[NUM_PORT];
int input_occupy[NUM_PORT], output_occupy[NUM_PORT];
int match[NUM_PORT]; //input[match[i]]--output[i];
int matchx[NUM_PORT];
int slack[NUM_PORT]; //各输出端口还差多少流量可以匹配上一个输入
int visy[NUM_PORT];

int search_augmenting_path(const int *demand, int present_left);
int maximum_weight_matching(const int *demand, int *result);

//int dfs(int* demand, int x);
//int hungary(int* demand, int *result);

#endif