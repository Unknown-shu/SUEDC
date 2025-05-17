#ifndef FILERT_H
#define FILERT_H

#include "zf_common_typedef.h"
#include "zf_common_debug.h"

#define my_assert(x)        zf_assert(x)

//滑动滤波器
typedef struct {
    float *buffer;                 // 数据缓冲区
    uint16 window_size;            // 窗口大小
    uint16 current_index;          // 当前索引位置
    uint16 data_count;             // 当前数据个数
    float sum;                     // 数据总和
    uint8 is_initialized;          // 初始化标志
} Sliding_Filter_t;

//低通滤波器
typedef struct {
    float alpha;                 // 滤波系数
    float last_output;           // 上一次的输出值
    uint8 is_initialized;        // 初始化标志
} LowPass_Filter_t;

extern Sliding_Filter_t ADC_filter;
extern Sliding_Filter_t vpp_filter;
extern LowPass_Filter_t ADC_LowPass_filter;

// 滑动滤波器初始化
uint8 Sliding_Filter_Init(Sliding_Filter_t *filter, uint16 window_size);

// 向滑动滤波器添加新数据
void Sliding_Filter_AddData(Sliding_Filter_t *filter, float new_data);

// 获取滑动滤波器的平均值
float Sliding_Filter_GetAverage(Sliding_Filter_t *filter);

// 释放滑动滤波器资源
void Sliding_Filter_Deinit(Sliding_Filter_t *filter);

// 低通滤波器初始化 - alpha越大滤波效果越强
uint8 LowPass_Filter_Init(LowPass_Filter_t *filter, float alpha, float initial_value);

// 向低通滤波器添加新数据并获取滤波结果
float LowPass_Filter_Update(LowPass_Filter_t *filter, float new_data);

// 获取低通滤波器的当前值
float LowPass_Filter_GetValue(LowPass_Filter_t *filter);

// 重置低通滤波器
void LowPass_Filter_Reset(LowPass_Filter_t *filter, float value);

// 设置低通滤波器系数
uint8 LowPass_Filter_SetAlpha(LowPass_Filter_t *filter, float alpha);


#endif
