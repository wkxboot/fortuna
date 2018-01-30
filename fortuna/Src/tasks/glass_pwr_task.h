#ifndef  __GLASS_PWR_TASK_H__
#define  __GLASS_PWR_TASK_H__
#include "ABDK_ZNHG_ZK.h"
/*玻璃加热电源任务*/
void glass_pwr_task(void const * argument);
extern osThreadId glass_pwr_task_hdl;
/*获取加热玻璃状态*/
bsp_state_t get_glass_pwr_state();

/*玻璃加热任务运行间隔.不需要实时响应.所以间隔大些*/
#define  GLASS_PWR_TASK_INTERVAL               200
/*玻璃加热任务工作最大时长 5分钟*/
#define  GLASS_PWR_TASK_WORK_TIME_MAX         (5*60*1000UL)
/*玻璃加热任务信号*/
#define  GLASS_PWR_TASK_ON_SIGNAL             (1<<0)
#define  GLASS_PWR_TASK_OFF_SIGNAL            (1<<1)
#define  GLASS_PWR_TASK_ALL_SIGNALS           ((1<<2)-1)

#endif