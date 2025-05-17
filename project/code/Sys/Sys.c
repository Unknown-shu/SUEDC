/*
 * Sys.c
 *
 *  Created on: 2024年10月27日
 *      Author: sun
 */
#include "Sys.h"
/***********************************************
* @brief : 计算绝对值
* @param : z
* @return: z
* @date  : 2024年11月4日10:06:55
* @author: SJX
************************************************/
float absolute(float z)
{
    z = z< 0 ? (-z) : z;
    return z;
}
/***********************************************
* @brief : 差比和
* @param : int16 a                   数值较大的灰度值
*          int16 b                   数值较小的灰度值
*          uint8 compare_value       差比和阈值
* @return:
* @date  : 2024年11月4日10:06:55
* @author: SJX
************************************************/
int16 Compare_Num(int16 a, int16 b, uint8 compare_value)
{
    if(a >= 10 && b >= 10)
    {
        if((((a - b) << 7) / (a + b)) > compare_value &&(((a - b) << 7) / (a + b)) < 128 )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
/***********************************************
* @brief : 三角滤波
* @param :
* uint8 *line 指向需要滤波的数据数组的指针
* uint8 start 起点
* uint8 end   终点
* @return:
* @date  : 2024年11月4日10:06:55
* @author: SJX
************************************************/
float Triangular_Filter_Weights[3] = {0.26, 0.48, 0.26};
void Triangular_Filter(uint8 *line, uint8 start, uint8 end)
{
    uint8 i = 0;
    for(i = start + 1; i < end - 1; i ++)
    {
        line[i] = (uint8)(Triangular_Filter_Weights[0] * (float)line[i - 1] + Triangular_Filter_Weights[1] * (float)line[i    ] + Triangular_Filter_Weights[2] * (float)line[i + 1]);
    }
}
/***********************************************
* @brief : 快速开方
* @param : number   值
* @return: float    平方根倒数
* @date  : 2025年3月16日16:03:543
* @author: STL
* @exp   :
************************************************/
float Q_rsqrt( float number )
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;                       // evil floating point bit level hacking
    i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
    y  = * ( float * ) &i;
    y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//    y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    return y;
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      pit初始化            //注意time是浮点型,单位为us
//  参数说明      pit_index           选择CCU6模块
//  参数说明      time                周期时间
//  返回参数      void
//  使用示例      pit_init(CCU60_CH0, 5000);      // 设置周期中断5000us
//  备注信息      请使用.h文件中 带时间单位的宏定义函数
//-------------------------------------------------------------------------------------------------------------------
void my_pit_init (pit_index_enum pit_index, float time)
{
    uint8 i;
    volatile Ifx_CCU6 *module;
    uint64 timer_input_clk;
    IfxCcu6_Timer g_Ccu6Timer;
    IfxCcu6_Timer_Config timerConfig;
    uint32 timer_period;

    boolean interrupt_state = disableInterrupts();

    module = IfxCcu6_getAddress((IfxCcu6_Index)(pit_index/2));

    IfxCcu6_Timer_initModuleConfig(&timerConfig, module);



    timer_input_clk = IfxScuCcu_getSpbFrequency();
    i = 0;
    while(i < 16)
    {
        timer_period = (uint32)(timer_input_clk * time / 1000000);
        if(timer_period < 0xffff)   break;
        timer_input_clk >>= 1;
        i++;
    }
    if(16 <= i) IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, FALSE);


    switch(pit_index)
    {
        case CCU60_CH0:
        {
            timerConfig.interrupt1.typeOfService  = CCU6_0_CH0_INT_SERVICE;
            timerConfig.interrupt1.priority       = CCU6_0_CH0_ISR_PRIORITY;
            break;
        }
        case CCU60_CH1:
        {
            timerConfig.interrupt2.typeOfService  = CCU6_0_CH1_INT_SERVICE;
            timerConfig.interrupt2.priority       = CCU6_0_CH1_ISR_PRIORITY;
            break;
        }
        case CCU61_CH0:
        {
            timerConfig.interrupt1.typeOfService  = CCU6_1_CH0_INT_SERVICE;
            timerConfig.interrupt1.priority       = CCU6_1_CH0_ISR_PRIORITY;
            break;
        }
        case CCU61_CH1:
        {
            timerConfig.interrupt2.typeOfService  = CCU6_1_CH1_INT_SERVICE;
            timerConfig.interrupt2.priority       = CCU6_1_CH1_ISR_PRIORITY;
            break;
        }
    }

    if((pit_index % 2) == 0)
    {
        timerConfig.timer = IfxCcu6_TimerId_t12;
        timerConfig.interrupt1.source          = IfxCcu6_InterruptSource_t12PeriodMatch;
        timerConfig.interrupt1.serviceRequest  = IfxCcu6_ServiceRequest_1;
        timerConfig.base.t12Period             = timer_period;
        timerConfig.base.t12Frequency          = (float)timer_input_clk;
        timerConfig.clock.t12countingInputMode = IfxCcu6_CountingInputMode_internal;
    }
    else
    {
        timerConfig.timer = IfxCcu6_TimerId_t13;
        timerConfig.interrupt2.source          = IfxCcu6_InterruptSource_t13PeriodMatch;
        timerConfig.interrupt2.serviceRequest  = IfxCcu6_ServiceRequest_2;
        timerConfig.base.t13Period             = timer_period;
        timerConfig.base.t13Frequency          = (float)timer_input_clk;
        timerConfig.clock.t13countingInputMode = IfxCcu6_CountingInputMode_internal;
    }
    timerConfig.timer12.counterValue     = 0;
    timerConfig.timer13.counterValue     = 0;
    timerConfig.trigger.t13InSyncWithT12 = FALSE;

    IfxCcu6_Timer_initModule(&g_Ccu6Timer, &timerConfig);

    restoreInterrupts(interrupt_state);

    Ifx_CPU_DBGSR debug_index;
    debug_index.U = __mfcr(CPU_DBGSR);
    if(1 == debug_index.B.DE)
    {
        IfxCcu6_setSuspendMode(module, IfxCcu6_SuspendMode_hard);
    }
    IfxCcu6_Timer_start(&g_Ccu6Timer);
}

