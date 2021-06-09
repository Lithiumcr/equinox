/**@file main.c
* @brief 日食算法实现
* @details cpu串行，计时
* @author 李昶融 harrylcr@qq.com
* @date 2021-6-1
* @version 11.3
* @attention
*
*/

#include <stdio.h>
#include <stdlib.h>
#include "mytimer.hpp"
#include "parameters.h"
#include "matbingen.h"
#include "matrixlcr.h"
#include "pretreat.h"
#include "mwm.h"
#include "config.h"

int main(int argc, char *argv[])
{
	/// 输入参数定义
	if (argc < 2)
		fprintf(stderr, "Equinox Version %s\n", PROJECT_VERSION_FULL);

	int epoch = atoi(argv[1]);
	/// window参数应考虑自适应调节，满足对时延或利用率的不同追求
#ifndef window
	int window = atoi(argv[2]);
#endif
	int output_every_turn = 0;
	if (argc >= 4)
		output_every_turn = atoi(argv[3]);

	/// 异步调度单位定义
	const int phase_length = window / NUM_OCS;

	/// 循环变量定义
	int iter = 0;
	int index = 0;
	int index_low = 0, index_middle = 0, index_high = NUM_PORT * NUM_PORT;

	/// 需求矩阵、元素去重排序表、调度方案表、调度时隙表、重构计数、残余流量等定义
	int demand[NUM_PORT * NUM_PORT], demand_queue[NUM_PORT * NUM_PORT];
	int demand_restricted_1[NUM_PORT * NUM_PORT], demand_restricted_2[NUM_PORT * NUM_PORT], demand_devided[NUM_PORT * NUM_PORT];
	int sorted_terms[NUM_PORT * NUM_PORT] = {0};
	//int schedule[NUM_PORT * NUM_PORT << 1];
	//memset(schedule, -1, NUM_PORT * NUM_PORT * 2 * sizeof(int));
	int candidate_match[NUM_OCS * NUM_PORT + NUM_PORT] = {0};
	int duration[NUM_PORT * NUM_OCS] = {0};
	//int occupied_slot = 0;
	int reconfiguration_counter = 0, total_reconfiguration_counter = 0;
	int this_OCS = 0;
	int duty_schedule[NUM_OCS];
	for (index = 0; index < NUM_OCS; ++index)
		index[duty_schedule] = index; //生成循环轮值表
	int occupied_slot_branch[NUM_OCS] = {0};
	int mark = 0;
	int remain = 0, clean_flag = 0;
	long long last_remain = 0;
	int this_avail_slot = phase_length - RECONFIGURATION_DELAY;
	//print_matrix(duration,NUM_PORT,1);
	//memset(duration,0,NUM_PORT*sizeof(int));

	/// 持续时隙、最大权、有效利用率、OCS单次调度用时定义
	int dur1 = 0, dur2 = 0;
	long long mwm1 = 0LL, mwm2 = 0LL;
	long long total_mwm_OCS = 0;
	int mwm_EPS = 0;
	REAL eu1 = 0.0, eu2 = 0.0;
	int OCS_turn = 0;

	/// 整体仿真参数定义
	long long last_checkpoint = 0LL, simulated_slot = 0LL;
	int output_times = 0;
	REAL simulated_slot_ms = 0.0;
	REAL frequency_reconfig = 0.0;
	REAL mean_delay = 0.0;
	REAL OCS_utillization = 0.0;
	REAL t1, t2, elapsed;
	REAL fraction_total_traffic;
	long long thisturn_arrival_count = 0, total_arrival_count = 0;
	long long total_arrival_product = 0LL, total_departure_product = 0LL, thisturn_arrival_product = 0LL;

	/*
	/// 随机数表定义
	int poisson_table[1 << 16];
	FILE *fp_read = fopen("../poisson_table.dat", "r");
	fread(poisson_table, sizeof(int), 1 << 16, fp_read);
	fclose(fp_read);
	*/

	/// 输出调度方案及统计量的文件
	FILE *fp_output = NULL;
	char outfilename[64];
	sprintf(outfilename, "./output-%do-%dp-%dδ-%dw-%de.txt", NUM_OCS, NUM_PORT, RECONFIGURATION_DELAY, window, epoch);
	fprintf(stderr, outfilename);
	fp_output = fopen(outfilename, "w");
	if (fp_output == NULL)
	{
		fprintf(stderr, "    open output.txt failed, Bye!");
		return -1;
	}

	//srand((int)time(NULL));
	srand(5);

	fprintf(stderr, "\nStart simulation.\n");
	//int total_occupied_slot = 0; //unit: slot

	t1 = get_wall_time();
	// initialize demand_queue matrix;
	memset(demand_queue, 0, NUM_PORT * NUM_PORT * sizeof(int));
	thisturn_arrival_count = fine_poisson_arrival(demand_queue, phase_length, &thisturn_arrival_product);
	//print_matrix(demand_queue, NUM_PORT, NUM_PORT);

	for (iter = 1; iter <= epoch * NUM_OCS; ++iter)
	{
		/// 从上一轮的queue移入待分解的demand
		memcpy(demand, demand_queue, NUM_PORT * NUM_PORT * sizeof(int));
		memset(demand_queue, 0, NUM_PORT * NUM_PORT * sizeof(int));
		clean_flag = 0;
		total_arrival_count += thisturn_arrival_count;	   /// 总包到达累加
		total_arrival_product += thisturn_arrival_product; //以本轮开始检查点为参考0点
		thisturn_arrival_product = 0LL;
		/// 先安排EPS
		for (index = 0; index < NUM_PORT * NUM_PORT; ++index)
		{
			if (index[demand])
			{
				mwm_EPS = mu_EPS * phase_length; //可能无法清除足够多的小流导致极低效率的匹配？
				//mwm_EPS = mu_EPS * window;
				if (demand[index] < mwm_EPS)
				{
					mwm_EPS = demand[index];
					demand[index] = 0;
				}
				else
				{
					demand[index] -= mwm_EPS;
				}
				total_departure_product += ((long long)mwm_EPS * mwm_EPS * ON_PROPORTION / 2); //计入此轮EPS的贡献。如用int，此处可能溢出！
			}
		}
		///< 至此，demand 应极为稀疏

		/// get_OCS_schedule;
		//occupied_slot = 0;

		this_OCS = duty_schedule[(iter - 1) % NUM_OCS];
		reconfiguration_counter = 0; /// 该值应当每轮清零，否则引起duration下标越界，引发灾难性bug！
		memset(duration, 0, NUM_PORT * sizeof(int));
		occupied_slot_branch[this_OCS] = 0;
		while ((this_avail_slot = window - RECONFIGURATION_DELAY - occupied_slot_branch[this_OCS]) > 0)
		{
			index_low = 0;
			index_high = sort_deduplication(sorted_terms, demand, NUM_PORT * NUM_PORT);
			/// 提前计算窗长门限，纳入压平效应考虑
			index = index_high;
			while (sorted_terms[index] > this_avail_slot)
			{
				sorted_terms[index] = this_avail_slot;
				--index;
			}
			index_high = index + 1;

			//fprintf(stderr, "\nDeduplicated feasible solutions, ascending order:");			print_matrix(sorted_terms, index_high + 1, 1);
			if (sorted_terms[0] < 0)
			{
				fprintf(stderr, "Minus term error\n");
				return -1;
			}

			/// algorithm core;
			while (1)
			{
				if (index_high > index_low)
				{
					//fprintf(stderr, "\nBranch 1: index_high == %d > index_low == %d\n", index_high, index_low);
					index_middle = (index_high + index_low) >> 1; // 此句可去掉括号，实际上这也是原本的运算优先级
					dur1 = sorted_terms[index_middle];			  // 多机时，考虑持续时长只计算各OCS分到的部分
					dur2 = sorted_terms[index_middle + 1];
					//fprintf(stderr, "\nDuration: dur1==%d, dur2==%d\n", dur1, dur2);

					//分别计算压平效应
					for (index = 0; index < NUM_PORT * NUM_PORT; ++index)
						index[demand_restricted_1] = (index[demand] > dur1 ? dur1 : index[demand]);
					for (index = 0; index < NUM_PORT * NUM_PORT; ++index)
						index[demand_restricted_2] = (index[demand] > dur2 ? dur2 : index[demand]);
					//fprintf(stderr, "\nTraffic demand in the inside while loop 1, mwm1:\n");				print_matrix(demand_restricted_1,NUM_PORT,NUM_PORT);
					//fprintf(stderr, "\nTraffic demand in the inside while loop 1, mwm2:\n");				print_matrix(demand_restricted_2,NUM_PORT,NUM_PORT);
					//fprintf(stderr, "\nTraffic remain old: \n");		print_matrix(demand, NUM_PORT, NUM_PORT);

					mwm1 = maximum_weight_matching(demand_restricted_1, candidate_match + NUM_PORT * this_OCS); //主方案按this_OCS位置安排
					mwm2 = maximum_weight_matching(demand_restricted_2, candidate_match + NUM_PORT * NUM_OCS);	//备用方案暂存在最后一个额外位置上
					//mwm1 = hungary(demand_restricted_1, candidate_match);				mwm2 = hungary(demand_restricted_2, candidate_match + NUM_PORT);
					eu1 = (double)mwm1 / (dur1 + RECONFIGURATION_DELAY);
					eu2 = (double)mwm2 / (dur2 + RECONFIGURATION_DELAY);
					//fprintf(stderr, "\nThroughput: mwm1==%d, mwm2==%d\n", mwm1, mwm2);
					//fprintf(stderr, "\nEffective utilization: eu1==%f, eu2==%f\n", eu1, eu2);

					///< This branch architecture could be far optimized later, decreasing the time of calling maximum_weight_matching with the same dur;
					if (eu1 <= eu2)
					{
						if (index_high == index_middle + 1)
						{
							memcpy(candidate_match + NUM_PORT * this_OCS, candidate_match + NUM_PORT * NUM_OCS, NUM_PORT * sizeof(int));
							dur1 = dur2;
							break;
						}
						index_low = index_middle + 1;
					}
					else
					{
						if (index_high == index_middle + 1)
							break;
						index_high = index_middle;
					}
				}
				else
				{
					//fprintf(stderr, "\nBranch 2: index_high == index_low == %d\n", index_low);
					dur1 = sorted_terms[index_low];
					//fprintf(stderr, "\nDuration: dur==%d\n", dur1);
					for (index = 0; index < NUM_PORT * NUM_PORT; ++index)
						index[demand_restricted_1] = (index[demand] > dur1 ? dur1 : index[demand]);
					//fprintf(stderr, "\nTraffic demand in the inside while loop 2:\n");				print_matrix(demand_restricted_1, NUM_PORT, NUM_PORT);
					//fprintf(stderr, "\nTraffic remain old: \n");		print_matrix(demand, NUM_PORT, NUM_PORT);

					mwm1 = maximum_weight_matching(demand_restricted_1, candidate_match);
					break;
				}
			}
			transpose_sparse(candidate_match + NUM_PORT * this_OCS, NUM_PORT);

			duration[reconfiguration_counter] = dur1;
			++reconfiguration_counter;

			OCS_turn = dur1 + RECONFIGURATION_DELAY;
			total_mwm_OCS += mwm1;
			total_departure_product += mwm1 * (occupied_slot_branch[this_OCS] + (OCS_turn >> 1));
			//occupied_slot += OCS_turn;
			occupied_slot_branch[this_OCS] += OCS_turn;

			// 刷新流量矩阵
			for (index = 0; index < NUM_PORT; ++index)
			{
				mark = index * NUM_PORT + candidate_match[index + NUM_PORT * this_OCS];
				demand[mark] -= dur1;
				if (demand[mark] < 0)
					demand[mark] = 0;
				//remain = demand[mark] - dur1;
				//demand[mark] = (remain > 0 ? remain : 0);
			}
		}

		last_remain = (long long)sum_item(demand, NUM_PORT * NUM_PORT);
		fprintf(stderr, "Demand remain in this turn: %lld\n", last_remain);

		if (last_remain)
		{
			//fprintf(stderr, "\nTraffic not clean at epoch %d !\n", iter);
			//return 1;
			/// 未清空的写回到达队列，注意轮空罚时
			for (index = 0; index < NUM_PORT * NUM_PORT; ++index)
				if (index[demand])
				{
					index[demand_queue] += index[demand];
					index[demand] = 0;
				}
			total_departure_product += last_remain * phase_length; /// 小心此处乘法溢出
		}

		/// 在demand_queue中详细考虑本轮的包到达
		thisturn_arrival_count = fine_poisson_arrival(demand_queue, phase_length, &thisturn_arrival_product);
		last_checkpoint = (phase_length * iter); /// 异步调度时此步是将仿真时刻从虚的occupied_slot_branch时刻回退，定到待处理的相位平分点
		simulated_slot = last_checkpoint;

		/// calc_traffic_delay;
		mean_delay = (REAL)(total_departure_product - total_arrival_product) / total_arrival_count + phase_length;
		if (mean_delay <= 0)
		{
			fprintf(stderr, "\nHeavy backlog in demand, or huge number overflow! pseudo mean delay: %f\n", mean_delay);
			return 1;
		}
		else
		{
			total_departure_product -= total_arrival_product;
			total_arrival_product = 0LL;
		}

		///	calc_OCS_utillization;
		//total_occupied_slot += sum_item(occupied_slot_branch, NUM_OCS);
		total_reconfiguration_counter += reconfiguration_counter;
		OCS_utillization = total_mwm_OCS / (NUM_PORT * NUM_OCS * (REAL)(simulated_slot)); /// 此处必须先转换浮点，否则溢出
		///	calc_OCS_reconfiguration_rate;
		simulated_slot_ms = t0 * simulated_slot;
		frequency_reconfig = total_reconfiguration_counter / (NUM_OCS * simulated_slot_ms); /// unit: ms^(-1)
		//fprintf(stderr, "\nSimulation mean delay:\n%f slots\n", mean_delay);
		mean_delay *= t0; /// unit: ms

		if ((output_every_turn) || (simulated_slot_ms >= output_times * 100))
		{
			++output_times;
			fprintf(stderr, "\nOCS count till epoch %d:\n%lld\n", iter, total_mwm_OCS);
			fprintf(fp_output, "\nOCS count till epoch %d:\n%lld\n", iter, total_mwm_OCS);
			fprintf(stderr, "\nTotal packet count till epoch %d:\n%lld\n", iter, total_arrival_count);
			fprintf(fp_output, "\nTotal packet count till epoch %d:\n%lld\n", iter, total_arrival_count);
			fprintf(stderr, "\nSimulation mean delay till epoch %d:\n%f ms\n", iter, mean_delay);
			fprintf(fp_output, "\nSimulation mean delay till epoch %d:\n%f ms\n", iter, mean_delay);
			fprintf(stderr, "\nOCS_utillization:\n%f\n", OCS_utillization);
			fprintf(fp_output, "\nOCS_utillization:\n%f\n", OCS_utillization);
			//fprintf(stderr, "\nTotal reconfiguration count:\n%d\n", total_reconfiguration_counter);
			fprintf(stderr, "\nSystem time:\n%f ms\n", simulated_slot_ms);
			fprintf(fp_output, "\nSystem time:\n%f ms\n", simulated_slot_ms);
			fprintf(stderr, "\nFrequency_reconfig:\n%f /ms\n", frequency_reconfig);
			fprintf(fp_output, "\nFrequency_reconfig:\n%f /ms\n", frequency_reconfig);
			fprintf(stderr, "\n--------%d ends--------\n", iter);
			fprintf(fp_output, "\n--------%d ends--------\n", iter);
		}
	}

	t2 = get_wall_time();
	elapsed = t2 - t1;
	//get final statistics, output;
	fprintf(stderr, "\nSimulation elapsed time: %f seconds, simulation eff: %f ms/s\n",
			elapsed, simulated_slot_ms / elapsed);
	fprintf(fp_output, "\nSimulation elapsed time: %f seconds, simulation eff: %f ms/s\n",
			elapsed, simulated_slot_ms / elapsed);
	fclose(fp_output);
	return 0;
}
