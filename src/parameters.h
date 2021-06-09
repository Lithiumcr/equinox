#ifndef _PARA_H
#define _PARA_H

typedef double REAL;

#define NUM_OCS 4
#define NUM_PORT 80                 //OCS端口数
#define RECONFIGURATION_DELAY 15625 //重构开销，1 ms = 15625 slot
#define OCSbitrate 100 * 1e9        //bps
#define packetsize 6400             //bit
//#define window 1562500          //归一化时域窗长
//#define window 100 * RECONFIGURATION_DELAY //归一化时域窗长
#define t0 64e-6 // unit:ms，归一化时隙长度，packetsize/OCSbitrate

#define ON_PROPORTION 61     //高阶段占总周期时间均值的倒数，默认为61，即高阶段:低阶段=1:60
#define alpha t0 / (500 *1)      //高阶段持续时间均值的倒数，相当于1/(0.5 s)，归一化即128e-9 /slot
#define beta t0 / (30000 *1)    //低阶段持续时间均值的倒数，相当于1/(30 s)，归一化即2.13e-9 /slot
#define lambda_on 0.9 /1       //高阶段到达率 = 90 Gb/s，归一化即 0.9 pkt/slot
#define lambda_off 1.0 / 600/1 //低阶段到达率 = 1.0/6 Gb/s，归一化即 1.67E-3 pkt/slot

// b=0.5
#define mu_OCS 1                                                              //OCS服务率 = 100Gb/s = 15.625 pkt/us，以此归一化即 1 pkt/slot
#define mu_EPS (lambda_on + lambda_off * (ON_PROPORTION - 1)) / ON_PROPORTION //EPS服务率，到达率对高低阶段时长加权平均，归一化即 1/61 pkt/slot，刚好等于理论平均到达率
//const int defalt_refresh = (int)(1.0 / mu_EPS);                       //上式的倒数，便于计算，恰为61
#define threshold 111200 // unit: pkt，即88.96MB

#define MAX 0x3f3f3f3f // 用于mwm.c

#endif
