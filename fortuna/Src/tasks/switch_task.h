#ifndef  __SWITCH_TASK_H__
#define  __SWITCH_TASK_H__

/*按键任务*/
void switch_task(void const * argument);
extern osThreadId switch_task_hdl;

#define  SWITCH_TASK_INTERVAL                      40 /*按键监视间隔40ms*/
#define  SWITCH_STATE_MONITOR_DELAY                10 /*按键抖动时间*/

#define  SWITCH_TASK_INIT_DISPLAY_HOLD_ON_TIME     250/*初始化显示时数据保持时间*/

#define  SWITCH_TASK_WAIT_TIMEOUT                  100/*按键任务操作电子秤等待超时时间*/
#define  SWITCH_TASK_CALIBRATE_EXIT_WAIT_TIME      2000/*等待校准退出的时间*/








#endif