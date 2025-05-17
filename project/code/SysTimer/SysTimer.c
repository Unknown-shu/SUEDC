#include "SysTimer.h"

uint32 timer_start_time, timer_stop_time, g_past_time;

void SysTimer_Start(void)
{
    timer_start_time = MY_GET_TIME();
//    timer_start_time = g_timer_1ms_cnt;
}

void SysTimer_Stop(void)
{
    timer_stop_time = MY_GET_TIME();
    g_past_time = timer_stop_time - timer_start_time;
}

uint32 GetPastTime(void)
{
    g_past_time = timer_stop_time - timer_start_time ;
    return g_past_time;
}
