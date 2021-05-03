#include "mwm.h"

int search_augmenting_path(const int *demand, int present_left)
{
	//fprintf(stderr, "Here, search path\n");
	input_occupy[present_left] = 1;
	for (int present_right = 0; present_right < NUM_PORT; ++present_right)
	{
		if (output_occupy[present_right])
			continue;

		int gap = pole_input[present_left] + pole_output[present_right] - demand[present_left * NUM_PORT + present_right];
		if (gap)
		{
			if (gap < slack[present_right])
				slack[present_right] = gap;
			//fprintf(stderr, "gap==%d, slack[%d]==%d\n", gap, present_right, slack[present_right]);
		}
		else
		//gap符合要求
		{
			output_occupy[present_right] = 1;
			//fprintf(stderr, "present_left==%d, present_right==%d, match[present_right]==%d\n", present_left, present_right, match[present_right]);
			//找一个空闲、或能腾出来的输出端口
			//注意此处可能会导致死循环递归调用
			if ((match[present_right] < 0) || (search_augmenting_path(demand, match[present_right])))
			{
				match[present_right] = present_left;
				//fprintf(stderr, "%d--%d\n", present_left, present_right);
				return 1;
			}
		}
	}
	return 0;
}

int maximum_weight_matching(const int *demand, int *result)
{
	int mwm = 0, i = 0;
	memset(match, -1, sizeof(match));
	memset(pole_output, 0, sizeof(pole_output));

	for (i = 0; i < NUM_PORT; ++i)
	{
		result[i] = maximum_element_id(demand + i * NUM_PORT, NUM_PORT);
		pole_input[i] = demand[NUM_PORT * i + result[i]];
		//pole_input[i] = demand[NUM_PORT * i + maximum_element_id(demand + i * NUM_PORT, NUM_PORT)];
	}
	//print_matrix(result,NUM_PORT,1);    print_matrix(pole_input,NUM_PORT,1);

	//give each input port a match;
	for (int present_input_port = 0; present_input_port < NUM_PORT; ++present_input_port)
	{
		memset(slack, MAX, sizeof(slack));
		//fprintf(stderr, "\nslack:\n");        print_matrix(slack, NUM_PORT, 1);
		while (1)
		{
			memset(output_occupy, 0, sizeof(output_occupy));
			memset(input_occupy, 0, sizeof(input_occupy));
			if (search_augmenting_path(demand, present_input_port))
				break; //唯一正常退出while死循环的分支

			//backtrack;
			//calculate inc=min{pole_input[x]+pole_output[y]-demand[x,y]} and get corresponding x,y;
			int inc = MAX;
			int miny;
			//注意此处循环变量用j，是为了和外层的i相区分
			//for (int j = 0; j < NUM_PORT; ++j)
			//if (input_occupy[j])
			for (int present_output_port = 0; present_output_port < NUM_PORT; ++present_output_port)
				if (!output_occupy[present_output_port])
				{
					if (slack[present_output_port] < inc)
						inc = slack[present_output_port];
					//inc = min(inc,slack[present_output_port]);
					miny = present_output_port;
					//input[j]--output[present_output_port];
				}

			if (!inc)
			{
				fprintf(stderr, "fatal error. inc==0\n");
				print_matrix(match, NUM_PORT, 1);
				fprintf(stderr, "input and output_occupy:");
				print_matrix(input_occupy, NUM_PORT, 1);
				print_matrix(output_occupy, NUM_PORT, 1);

				fprintf(stderr, "present min y==%d\n", miny);
				search_augmenting_path(demand, match[miny]);
				match[miny] = -1;
				output_occupy[miny] = 0;

				exit(0);
				//break;
			}

			for (int j = 0; j < NUM_PORT; ++j)
			{
				if (input_occupy[j])
					pole_input[j] -= inc;
				if (output_occupy[j])
					pole_output[j] += inc;
				else
					slack[j] -= inc;
			}
			//fprintf(stderr, "Here!\n");
		}
	}
	//输出
	memcpy(result, match, NUM_PORT * sizeof(int));
	for (i = 0; i < NUM_PORT; ++i)
		if (match[i] >= 0)
			mwm += demand[match[i] * NUM_PORT + i];

	return mwm;
}