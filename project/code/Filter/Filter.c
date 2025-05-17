#include "filter.h"

//===================滑动滤波器相关定义===================
#define SLIDING_FILTER_MAX_SIZE 10000                       //窗口最大值(实际上没什么用)

Sliding_Filter_t ADC_filter;
Sliding_Filter_t vpp_filter;
LowPass_Filter_t ADC_LowPass_filter;

/***********************************************
* @brief : 滑动滤波器初始化
* @param : filter            滤波器对象指针
* @param : window_size       滑动窗口大小
* @return: uint8             初始化是否成功，1成功，0失败
* @date  : 2025年5月12日16:12:45
* @author: AI Copilot
************************************************/
uint8 Sliding_Filter_Init(Sliding_Filter_t *filter, uint16 window_size)
{
    my_assert(filter != NULL);
    
    if(window_size == 0 || window_size > SLIDING_FILTER_MAX_SIZE)
    {
        return 0;  // 窗口大小无效
    }
    
    filter->buffer = (float *)malloc(window_size * sizeof(float));
    if(filter->buffer == NULL)
    {
        return 0;  // 内存分配失败
    }
    
    filter->window_size = window_size;
    filter->current_index = 0;
    filter->data_count = 0;
    filter->sum = 0.0f;
    filter->is_initialized = 1;
    
    // 缓冲区初始化为0
    memset(filter->buffer, 0, window_size * sizeof(float));
    
    return 1;
}

/***********************************************
* @brief : 向滑动滤波器添加新数据
* @param : filter            滤波器对象指针
* @param : new_data          新的数据值
* @return: void
* @date  : 2025年5月12日16:12:45
* @author: AI Copilot
************************************************/
void Sliding_Filter_AddData(Sliding_Filter_t *filter, float new_data)
{
    my_assert(filter != NULL);
    my_assert(filter->is_initialized == 1);
    
    // 减去即将被替换的数据
    if(filter->data_count == filter->window_size)
    {
        filter->sum -= filter->buffer[filter->current_index];
    }
    else
    {
        filter->data_count++;
    }
    
    // 添加新数据
    filter->buffer[filter->current_index] = new_data;
    filter->sum += new_data;
    
    // 移动索引指针
    filter->current_index = (filter->current_index + 1) % filter->window_size;
}

/***********************************************
* @brief : 获取滑动滤波器的平均值
* @param : filter            滤波器对象指针
* @return: float             滤波后的平均值
* @date  : 2025年5月12日16:12:45
* @author: AI Copilot
************************************************/
float Sliding_Filter_GetAverage(Sliding_Filter_t *filter)
{
    my_assert(filter != NULL);
    my_assert(filter->is_initialized == 1);
    
    if(filter->data_count == 0)
    {
        return 0.0f;
    }
    
    return filter->sum / filter->data_count;
}

/***********************************************
* @brief : 释放滑动滤波器资源
* @param : filter            滤波器对象指针
* @return: void
* @date  : 2025年5月12日16:12:45
* @author: AI Copilot
************************************************/
void Sliding_Filter_Deinit(Sliding_Filter_t *filter)
{
    my_assert(filter != NULL);
    
    if(filter->is_initialized && filter->buffer != NULL)
    {
        free(filter->buffer);
        filter->buffer = NULL;
        filter->is_initialized = 0;
    }
}

/***********************************************
* @brief : 低通滤波器初始化
* @param : filter            滤波器对象指针
* @param : alpha             滤波系数(0<alpha<=1)，越小滤波效果越强
* @param : initial_value     初始输出值
* @return: uint8             初始化是否成功，1成功，0失败
* @date  : 2025年5月13日09:15:36
************************************************/
uint8 LowPass_Filter_Init(LowPass_Filter_t *filter, float alpha, float initial_value)
{
    my_assert(filter != NULL);
    
    if(alpha < 0.0f || alpha > 1.0f)
    {
        return 0;  // 滤波系数范围错误
    }
    
    filter->alpha = alpha;
    filter->last_output = initial_value;
    filter->is_initialized = 1;
    
    return 1;
}

/***********************************************
* @brief : 向低通滤波器添加新数据并获取滤波结果
* @param : filter            滤波器对象指针
* @param : new_data          新的数据值
* @return: float             滤波后的结果
* @date  : 2025年5月13日09:16:25
************************************************/
float LowPass_Filter_Update(LowPass_Filter_t *filter, float new_data)
{
    my_assert(filter != NULL);
    my_assert(filter->is_initialized == 1);
    
    // 应用修改后的低通滤波公式: y(n) = (1-α)*x(n) + α*y(n-1)
    // 当α=0时，输出=新输入(不滤波)
    // 当α=1时，输出=上次输出(完全滤波)
    float output = (1.0f - filter->alpha) * new_data + filter->alpha * filter->last_output;
    
    // 更新上一次的输出值
    filter->last_output = output;
    
    return output;
}

/***********************************************
* @brief : 获取低通滤波器的当前值
* @param : filter            滤波器对象指针
* @return: float             滤波器当前值
* @date  : 2025年5月13日09:17:02
************************************************/
float LowPass_Filter_GetValue(LowPass_Filter_t *filter)
{
    my_assert(filter != NULL);
    my_assert(filter->is_initialized == 1);
    
    return filter->last_output;
}

/***********************************************
* @brief : 重置低通滤波器
* @param : filter            滤波器对象指针
* @param : value             重置值
* @return: void
* @date  : 2025年5月13日09:17:23
************************************************/
void LowPass_Filter_Reset(LowPass_Filter_t *filter, float value)
{
    my_assert(filter != NULL);
    my_assert(filter->is_initialized == 1);
    
    filter->last_output = value;
}

/***********************************************
* @brief : 设置低通滤波器系数
* @param : filter            滤波器对象指针
* @param : alpha             新的滤波系数(0<alpha<=1)
* @return: uint8             设置是否成功，1成功，0失败
* @date  : 2025年5月13日09:17:46
************************************************/
uint8 LowPass_Filter_SetAlpha(LowPass_Filter_t *filter, float alpha)
{
    my_assert(filter != NULL);
    my_assert(filter->is_initialized == 1);
    
    if(alpha < 0.0f || alpha > 1.0f)
    {
        return 0;  // 滤波系数范围错误
    }
    
    filter->alpha = alpha;
    return 1;
}
