//#pragma once

#ifndef _MATBINGEN_H
#define _MATBINGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parameters.h"

#define PI 3.1415926535

double rand_exp(const double lambda);

long long fine_poisson_arrival(int *demand, const int t, long long *outside_product);

int rand_poisson(const REAL lambda);

int read_poisson_table(const int *poisson_table);

long long approximate_poisson_arrival(int *demand, const int t);

int poisson_arrival(int *demand, const int refresh_interval);

int long_poisson_arrival(int *d, const int t, const int refresh, int simulated_slot, int total_arrival_product);

double rand_std_normal();

double rand_normal(double mu, double sigma);

void shuffle(int *terms);

int large_small_flow(int *buffer, const int number_large, const int coefficient_large, const int number_small, const int coefficient_small);

#endif
