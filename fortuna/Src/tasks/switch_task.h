#ifndef  __SWITCH_TASK_H__
#define  __SWITCH_TASK_H__

/*按键任务*/
void switch_task(void const * argument);
extern osThreadId switch_task_hdl;
#define  SWICTH_TASK_INTERVAL                      40 /*按键监视间隔40ms*/
#define  SWITCH_STATE_MONITOR_DELAY                10 /*按键抖动时间*/

#define  INIT_DISPLAY_HOLD_ON_TIME                 250/*初始化显示时数据保持时间*/










#endif