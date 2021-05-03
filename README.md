# Equinox

Equinox是一个C语言编写的光电混合数据中心交换机调度算法模拟器，用于仿真交换机卸载突发流量的问题。

## 依赖
cmake-3.0 或以上

## 安装

1. 根据运行环境修改根目录下的`CMakeLists.txt`；

2. 在根目录下创建安装子目录（例如，`build`）并进入；

3. 使用`cmake .. && make`安装；

    如需使用gdb工具调试，请加编译选项：`cmake -DCMAKE_BUILD_TYPE="Debug" .. && make`

    如需获得最佳性能，请加编译选项：`cmake -DCMAKE_BUILD_TYPE="Release" .. && make`

## 运行

```bash
cd ${project_src}/bin
./equinox ${epoch} ${window} ${output_every_turn}
```

## 输入参数示例
其中，`${epoch}`是仿真迭代次数，int类型，`${window}`是归一化时间窗长，int类型，`${output_every_turn}`是是否每轮迭代都输出结果，int类型。

如要改变交换机端口数、交换机数目、数据包大小、重构开销等参数，应在`parameter.h`的宏定义中预设：

```c
#define NUM_OCS 6
#define NUM_PORT 80
```

然后再编译运行。

## 样例输出

```bash
# ./equinox 10 62500 1
# output-6o-80p-15625δ-125000w-10e.txt

OCS count till epoch 1:
10660909

Total packet count till epoch 1:
13279675

Simulation mean delay till epoch 1:
7.308598 ms

OCS_utillization:
0.177682

System time:
8.000000 ms

Frequency_reconfig:
1.500000 /ms

--------1 ends--------

OCS count till epoch 2:
19492797

Total packet count till epoch 2:
28471406

Simulation mean delay till epoch 2:
6.850398 ms

OCS_utillization:
0.162440

System time:
16.000000 ms

Frequency_reconfig:
1.250000 /ms

--------2 ends--------

OCS count till epoch 3:
23887869

Total packet count till epoch 3:
41031683

Simulation mean delay till epoch 3:
6.444434 ms

OCS_utillization:
0.132710

System time:
24.000000 ms

Frequency_reconfig:
1.333333 /ms

--------3 ends--------

OCS count till epoch 4:
34197312

Total packet count till epoch 4:
53623737

Simulation mean delay till epoch 4:
6.658126 ms

OCS_utillization:
0.142489

System time:
32.000000 ms

Frequency_reconfig:
1.343750 /ms

--------4 ends--------

OCS count till epoch 5:
38841477

Total packet count till epoch 5:
69259339

Simulation mean delay till epoch 5:
6.349533 ms

OCS_utillization:
0.129472

System time:
40.000000 ms

Frequency_reconfig:
1.375000 /ms

--------5 ends--------

OCS count till epoch 6:
50312213

Total packet count till epoch 6:
85503647

Simulation mean delay till epoch 6:
6.460721 ms

OCS_utillization:
0.139756

System time:
48.000000 ms

Frequency_reconfig:
1.312500 /ms

--------6 ends--------

OCS count till epoch 7:
58558641

Total packet count till epoch 7:
99747466

Simulation mean delay till epoch 7:
6.471552 ms

OCS_utillization:
0.139425

System time:
56.000000 ms

Frequency_reconfig:
1.250000 /ms

--------7 ends--------

OCS count till epoch 8:
69886205

Total packet count till epoch 8:
113178110

Simulation mean delay till epoch 8:
6.585616 ms

OCS_utillization:
0.145596

System time:
64.000000 ms

Frequency_reconfig:
1.234375 /ms

--------8 ends--------

OCS count till epoch 9:
77511884

Total packet count till epoch 9:
126606098

Simulation mean delay till epoch 9:
6.564002 ms

OCS_utillization:
0.143541

System time:
72.000000 ms

Frequency_reconfig:
1.194444 /ms

--------9 ends--------

OCS count till epoch 10:
85099609

Total packet count till epoch 10:
139680931

Simulation mean delay till epoch 10:
6.549152 ms

OCS_utillization:
0.141833

System time:
80.000000 ms

Frequency_reconfig:
1.250000 /ms

--------10 ends--------

Simulation elapsed time: 13.291653 seconds, simulation eff: 6.018815 ms/s

```

