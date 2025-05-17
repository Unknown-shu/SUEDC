#ifndef FFT_H
#define FFT_H

#include "zf_common_typedef.h"
#include "zf_common_debug.h"
#include <math.h>
#include "Beep.h"

#pragma section all "cpu1_dsram"

#define ADC_SAMPLE_SIZE    3200   // 采集数组大小
#define SAMPLING_FREQ      160000  // 采样频率
#define PI                 3.1415926535f

// 寻找最大的几个幅值及其频率
#define MAX_PEAKS 10  // 寻找的峰值数量

extern uint8  send_flag;                             // 采集完成标志
extern uint16 adc_sample_buffer[];  // ADC采样缓冲区
extern uint16 adc_sample_index;                      // 当前采样索引
extern float  main_frequency;                        //频率
extern float  vpp;                                   //峰峰值
extern uint16 wave_type;

extern float  main_frequency_protect_max ; // 保护频率最大值      
extern float  main_frequency_protect_min ;    // 保护频率最小值
extern float  vpp_protect_max ; // 保护幅值最大值
extern float  vpp_protect_min ; // 保护幅值最小值     

extern float fundamental_amp ; // 基波幅值
extern float second_harmonic ;    // 二次谐波
extern float third_harmonic  ;     // 三次谐波

// 查找谐波幅值 (允许±5%的频率偏差)
extern float harmonic2_range_min ;
extern float harmonic2_range_max ;
extern float harmonic3_range_min ;
extern float harmonic3_range_max ;

extern float second_ratio ; // 二次谐波幅值比
extern float third_ratio  ; // 三次谐波幅值比

extern float fifth_ratio  ; // 四次谐波幅值比

extern float SQUARE_third_ratio_min ;
extern float SQUARE_fifth_ratio_min ;
extern float TRIANGLE_third_ratio_min ;
extern float TRIANGLE_fifth_ratio_max ;
extern float SINE_third_ratio_max ;
extern float SINE_fifth_ratio_max ;

// 定义复数结构体，包含实部和虚部
typedef struct {
    double real;
    double image;
} Complex;

// 波形类型枚举
typedef enum {
    SINE_WAVE,      // 正弦波
    TRIANGLE_WAVE,  // 三角波
    SQUARE_WAVE,    // 方波
    UNKNOWN_WAVE    // 无法识别
} WaveType;

typedef struct {
    double amplitude;
    double frequency;
    uint16 index;
} PeakInfo;

void Test(void);
void DFT_Measure(void);
void Sys_Protect_Func(void);
void Vpp_Cal(void);

#pragma section all restore
#endif
