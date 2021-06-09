//生成二进制demand.bin
#include "matbingen.h"

double rand_exp(const double lambda)
{
	double res = (1.0 + rand()) / (RAND_MAX + 2.0); // res~U(0,1)
	res = (-1.0 / lambda) * log(res);
	return res;
}

long long fine_poisson_arrival(int *demand, const int t, long long *outside_product)
{
	REAL inside_slot = 0.0;
	REAL local_arrival_product = 0.0;
	REAL next_arrival[NUM_PORT * NUM_PORT] = {0.0}, next_flow_switch[NUM_PORT * NUM_PORT] = {0.0};
	long long packet_count = 0;
	int islow[NUM_PORT * NUM_PORT];

	//#pragma unroll(4)
	for (int port_iter = 0; port_iter < NUM_PORT * NUM_PORT; ++port_iter)
	{
		if (port_iter % (NUM_PORT + 1)) // (n+1)必然整除nxn矩阵的主对角线编号
		{
			islow[port_iter] = (rand() % 61 ? 1 : 0);
			inside_slot = 0.0;
			while (inside_slot < t)
			{
				/// 利用指数分布之无记忆性
				next_flow_switch[port_iter] = rand_exp((islow[port_iter] ? beta : alpha));
				next_arrival[port_iter] = rand_exp((islow[port_iter] ? lambda_off : lambda_on));
				if (next_arrival[port_iter] < next_flow_switch[port_iter])
				{
					++packet_count;
					++demand[port_iter];
					inside_slot += next_arrival[port_iter];
					local_arrival_product += inside_slot;
					continue;
				}
				islow[port_iter] = !islow[port_iter];
				inside_slot += next_flow_switch[port_iter];
			}
		}
	}
	*outside_product += (long long)local_arrival_product;
	return packet_count;
}

int rand_poisson(const double lambda)
{
	int x = 0;
	double p = 1.0;
	double cut = exp(-lambda);

	while (p >= cut)
	{
		double u = rand() / (RAND_MAX + 1.0);
		p *= u;
		++x;
	}
	return (x - 1);
}

int read_poisson_table(const int *poisson_table)
{
	int i = rand() % (1 << 16);
	return i[poisson_table];
}

long long approximate_poisson_arrival(int *demand, const int t)
{
	double total_packet = 0.0;
	int poisson_count = 0;
	for (int port_iter = 0; port_iter < NUM_PORT * NUM_PORT; ++port_iter)
	{
		if (port_iter % (NUM_PORT + 1)) // (n+1)必然整除nxn矩阵的主对角线编号
		{
			/*
			if (rand() % ON_PROPORTION)
				poisson_count = (int)rand_normal(lambda_off * t, lambda_off * t);
			else
				poisson_count = (int)rand_normal(lambda_on * t, lambda_on * t);
				*/
			poisson_count = (int)rand_normal(lambda_off * t, mu_EPS * t);

			if (poisson_count < 0) // 正态模拟可能导致负值，此时改为0
				poisson_count = 0;

			demand[port_iter] += poisson_count;
			total_packet += poisson_count;
		}
		else // 输入和输出是同一端口，即需求矩阵的主对角线，始终为0
			demand[port_iter] = 0;
	}
	//fprint_matrix(stderr, demand, NUM_PORT, NUM_PORT);
	return (int)total_packet;
}

int poisson_arrival(int *demand, const int t)
{
	int total_packet = 0;
	int poisson_count = 0;
	// 各端口随机确定当前处于高阶段或低阶段，分布按速率参数泊松分布确定包到达，刷新需求矩阵;
	for (int port_iter = 0; port_iter < NUM_PORT * NUM_PORT; ++port_iter)
	{
		if (port_iter % (NUM_PORT + 1)) // (n+1)必然整除nxn矩阵的主对角线编号
		{
			if (rand() % ON_PROPORTION)
				poisson_count = rand_poisson(lambda_off * t);
			else
				poisson_count = rand_poisson(lambda_on * t);

			demand[port_iter] += poisson_count;
			total_packet += poisson_count;
		}
		else // 输入和输出是同一端口，即需求矩阵的主对角线，始终为0
			demand[port_iter] = 0;
	}
	//fprint_matrix(stderr, demand, NUM_PORT, NUM_PORT);
	return total_packet;
}

// 拆分多次循环能加速仿真泊松过程吗？
int long_poisson_arrival(int *d, const int t, const int refresh, int simulated_slot, int total_arrival_product)
{
	int poisson_times = 1, res = 0;
	int sum = 0, arrival_count = 0;
	poisson_times = t / refresh;
	res = t % refresh;
	for (int tmp = 0; tmp < poisson_times; --tmp)
	{
		arrival_count = poisson_arrival(d, refresh);
		sum += arrival_count;													  // 总包到达计数
		total_arrival_product += (simulated_slot + refresh >> 1) * arrival_count; // 求总包到达积
		simulated_slot += refresh;
	}
	arrival_count = poisson_arrival(d, res);
	sum += arrival_count;												  // 总包到达计数
	total_arrival_product += (simulated_slot + res >> 1) * arrival_count; // 求总包到达积
	simulated_slot += res;

	return sum;
}

double rand_std_normal()
{
	double rayleigh, theta;
	int phase = 0;
	double result;
	if (phase)
	{
		result = sqrt(-2.0 * log(rayleigh)) * cos(2.0 * PI * theta);
	}
	else
	{
		rayleigh = rand() / (RAND_MAX + 1.0);
		theta = rand() / (RAND_MAX + 1.0);
		result = sqrt(-2.0 * log(rayleigh)) * sin(2.0 * PI * theta);
	}
	phase = 1 - phase;
	return result;
}

double rand_normal(double mu, double sigma)
{
	return (mu + sigma * rand_std_normal());
}

void shuffle(int *terms)
{
	int i = 0, target = 0, tmp = 0;
	for (; i < NUM_PORT; ++i)
	{
		target = rand() % NUM_PORT;
		//swap(terms[i], terms[target]);
		tmp = terms[i];
		terms[i] = terms[target];
		terms[target] = tmp;
	}
}

int large_small_flow(int *buffer, const int number_large, const int coefficient_large, const int number_small, const int coefficient_small)
{
	int i = 0, j = 0;
	int tmplarge = coefficient_large / number_large, tmpsmall = coefficient_small / number_small;
	int total = coefficient_large + coefficient_small;
	//int terms[NUM_PORT * NUM_PORT];
	int terms[NUM_PORT];
	for (i = 0; i < NUM_PORT; ++i)
		terms[i] = i;

	for (i = 0; i < number_large; ++i)
	{
		shuffle(terms);
		for (j = 0; j < NUM_PORT; ++j)
			buffer[j * NUM_PORT + terms[j]] += tmplarge;
	}
	for (i = 0; i < number_small; ++i)
	{
		shuffle(terms);
		for (j = 0; j < NUM_PORT; ++j)
			buffer[j * NUM_PORT + terms[j]] += tmpsmall;
	}
	/*
	//add perturbation within given amplitude;
	int perturbation_sigma = total *3 / 1000;
	int debit, credit;
	for (i = 0; i < NUM_PORT; ++i)
	{
		buffer[i * NUM_PORT + terms[i]] += (int)(rand_std_normal()*perturbation_sigma);
	}
    */
	return total;
}

//独立测试使用
/*
//宏定义的NUMBER_PORT提供行列数,argv[1]提供模式选择（0为小规模指定，1为主对角线均匀加载，2为随机大小流加载）
int main(int argc, char *argv[])
{	
	if(argc<1)
	{
		printf("Insufficient parameters! Fail to generate matrix.");
		return -1;
	}
	
	int branch = 2;
	branch = atoi(argv[1]);
	int target = 0;
	char suffix[10];
	//printf("%s\n",suffix);
	sprintf(suffix, "%d", NUM_PORT);
	//printf("%s\n",suffix);
	char filename[16];
	sprintf(filename, "%s", "demand-");
	strcat(filename, suffix);
	strcat(filename, ".bin");
	printf("\nFilename: %s\n", filename);

	FILE *fp = NULL;
	fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		printf("\nOpen file %s failed!\n", filename);
		return -1;
	}

	int buffer[NUM_PORT * NUM_PORT];
	memset(buffer, 0, NUM_PORT * NUM_PORT * sizeof(int));
	if (branch == 0)
	{
		buffer[14] = buffer[11] = buffer[4] = buffer[1] = 16;
		buffer[13] = buffer[8] = buffer[7] = buffer[2] = 8;
	}
	else if (branch == 1)
	{
		for (target = 0; target < NUM_PORT; ++target)
			buffer[target * NUM_PORT + target] = 1000;
	}
	else if (branch == 2)
	{
		int tensity = large_small_flow(buffer, 4, 700, 12, 300);
		printf("\nTraffic tensity: %d\n", tensity);
	}
	else
	{
		printf("\nInvalid branch parameter!\n");
		return -1;
	}

	fwrite(buffer, sizeof(int), NUM_PORT * NUM_PORT, fp);
	printf("\nWriting completed.\n");

	fclose(fp);
	return 0;
}
*/
