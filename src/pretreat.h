//pretreat, including sort, deduplicate, stuff, etc
#ifndef _SORT_H
#define _SORT_H

#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>

//#define BASE 10

int maximum_element_id(const int *matrix, const int length)
{
	int index = 0, i = 0, max = matrix[0];
	for (; i < length; ++i)
		if (max < matrix[i])
		{
			max = matrix[i];
			index = i;
		}
	return index;
}

int maximum_element_value(const int *matrix, const int length)
{
	return maximum_element_id(matrix, length)[matrix];
}

int minimum_element_id(const int *matrix, const int length)
{
	int index = 0, i = 0, min = matrix[0];
	for (; i < length; ++i)
		if (min > matrix[i])
		{
			min = matrix[i];
			index = i;
		}
	return index;
}

int minimum_element_value(const int *matrix, const int length)
{
	return minimum_element_id(matrix, length)[matrix];
}

/// 升序判断函数
int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

/// 排序后去重
int sort_deduplication(int *sorted_terms, const int *data, const int n)
{
	int i = 0;
	int present_fill = 0;

	/// 调用标准库快速排序
	memcpy(sorted_terms, data, n * sizeof(int));
	qsort(sorted_terms, n, sizeof(int), cmpfunc);
	//for(i=0;i<n;++i)	printf("\n%d",sorted_terms[i]);

	for (i = 0; i < (n - 1); ++i)
	{
		if (!(sorted_terms[i]) || (sorted_terms[i] == sorted_terms[i + 1]))
		{
			//printf("\ni==%d, 0 or ==: %d",i,sorted_terms[i]);
			continue;
		}
		sorted_terms[present_fill] = sorted_terms[i];
		++present_fill;
	}
	sorted_terms[present_fill] = sorted_terms[n - 1]; //?
	memset(sorted_terms + present_fill + 1, 0, (n - present_fill - 1) * sizeof(int));
	return (present_fill); ///< this digit starts from 0;
}

int quick_stuff(int *d)
{
	int result[NUM_PORT * NUM_PORT];
	memcpy(result, d, NUM_PORT * NUM_PORT * sizeof(int));
	int row_collum_sum[2 * NUM_PORT] = {0}; //前n个放置行和，后n个放置列和，便于后续求最值
	int index = 0;

	for (; index < NUM_PORT * NUM_PORT; ++index)
	{ /// 求各项所在的行和、列和，全部遍历一次
		row_collum_sum[row(index)] += d[index];
		row_collum_sum[column(index) + NUM_PORT] += d[index];
	}

	/// max sum of this two array, marked as phi in LiuHe2015
	int phi = maximum_element_value(row_collum_sum, 2 * NUM_PORT); // 需求矩阵的直径
	int artificial_demand = 0;
	for (index = 0; index < NUM_PORT * NUM_PORT; ++index)
	{
		if (d[index] > 0)
		{
			artificial_demand = phi - (row_collum_sum[row(index)] > row_collum_sum[column(index) + NUM_PORT] ? row_collum_sum[row(index)] : row_collum_sum[column(index) + NUM_PORT]);
			d[index] += artificial_demand;
			row_collum_sum[row(index)] += artificial_demand;
			row_collum_sum[column(index) + NUM_PORT] += artificial_demand;
		}
	}
	for (index = 0; index < NUM_PORT * NUM_PORT; ++index)
	{
		if (!d[index])
		{
			artificial_demand = phi - (row_collum_sum[row(index)] > row_collum_sum[column(index) + NUM_PORT] ? row_collum_sum[row(index)] : row_collum_sum[column(index) + NUM_PORT]);
			d[index] += artificial_demand;
			row_collum_sum[row(index)] += artificial_demand;
			row_collum_sum[column(index) + NUM_PORT] += artificial_demand;
		}
	}
	return phi;
}

#endif
