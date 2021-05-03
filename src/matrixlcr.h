//#pragma once
#ifndef _MATLCR_H
#define _MATLCR_H
#include <stdio.h>
#include <stdlib.h>

/// index编号对应的行号，即输入端口号
#define row(index) index / NUM_PORT
/// index编号对应的列号，即输出端口号
#define column(index) index % NUM_PORT

void print_matrix(const int *matrix, int column, int row)
{
	int j = 0;
	for (int i = 0; i < row; ++i)
	{
		fprintf(stderr, "\n");
		for (j = 0; j < column; ++j)
			fprintf(stderr, "%d\t", matrix[i * column + j]);
	}
	fprintf(stderr, "\n");
	return;
}

void fprint_matrix(FILE *fp, const int *matrix, const int column, const int row)
{
	int j = 0;
	for (int i = 0; i < row; ++i)
	{
		fprintf(fp, "\n");
		for (j = 0; j < column; ++j)
			fprintf(fp, "%d\t", matrix[i * column + j]);
	}
	fprintf(fp, "\n");
	return;
}

int sum_item(const int *array, const int size)
{
	int sum = 0;
	for (int i = 0; i < size; ++i)
		sum += array[i];
	return sum;
}

int diameter(int *d)
{
	int max = 0, row_sum = 0;
	for (int i = 0; i < NUM_PORT-1; ++i)
	{
		row_sum = sum_item(d + NUM_PORT * i, NUM_PORT);
		if (row_sum > max)
			max = row_sum;
	}
	return max;
}

int is_busy(const int *d)
{
	return (maximum_element_value(d, NUM_PORT * NUM_PORT) > threshold ? 1 : 0); // 该阈值参考了异步光卸载算法
}

void print_permutation_matrix(const int *matrix, int length)
{
	fprintf(stderr, "\nPermutation matrix:");
	int j = 0, corresponding_output = 0;
	for (int i = 0; i < length; ++i)
	{
		corresponding_output = matrix[i];
		fprintf(stderr, "\n");
		for (j = 0; j < corresponding_output; ++j)
			fprintf(stderr, "0\t");
		fprintf(stderr, "1\t");
		for (j = corresponding_output + 1; j < length; ++j)
			fprintf(stderr, "0\t");
	}
	fprintf(stderr, "\n");
	return;
}

void fprint_permutation_matrix(FILE *fp, const int *matrix, int length)
{
	fprintf(fp, "\nPermutation matrix:");
	int j = 0, corresponding_output = 0;
	for (int i = 0; i < length; ++i)
	{
		corresponding_output = matrix[i];
		fprintf(fp, "\n");
		for (j = 0; j < corresponding_output; ++j)
			fprintf(fp, "0\t");
		fprintf(fp, "1\t");
		for (j = corresponding_output + 1; j < length; ++j)
			fprintf(fp, "0\t");
	}
	fprintf(fp, "\n");
	return;
}

void transpose_sparse(int *matrix, const int wide)
{
	int temp[wide], i = 0;

	for (; i < wide; ++i)
	{
		temp[matrix[i]] = i;
	}
	memcpy(matrix, temp, wide * sizeof(int));
	//memcpy(matrix,temp,sizeof(temp));
	return;
}

//useless;
/*
int is_repeat(int *arr, int wide)
{
  int *hash[wide];
  memset(hash,0,wide);
  int i=0;
  for (;i<wide;++i)
  {
	if (hash[arr[i]])
	  return 1;
	hash[arr[i]] = 1;
  }
  return 0;
}
*/
#endif