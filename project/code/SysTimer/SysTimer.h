#ifndef __SYS_TIMER_
#define __SYS_TIMER_

#include "zf_driver_timer.h"

#define MY_GET_TIME()      system_getval_ns()

extern uint32 g_past_time;

void SysTimer_Start(void);
void SysTimer_Stop(void);
uint32 GetPastTime(void);

#endif
