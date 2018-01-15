#ifndef  __LIGHT_TASK_H__
#define  __LIGHT_TASK_H__


/*灯带任务*/
void light_task(void const * argument);
extern osThreadId light_task_hdl;



#define  LIGHT_TASK_INTERVAL               2/*2毫秒运行间隔*/

/*灯条任务信号*/
#define  LIGHT_TASK_LIGHT_1_PWR_ON_SIGNAL      (1<<0)
#define  LIGHT_TASK_LIGHT_1_PWR_OFF_SIGNAL     (1<<1)
#define  LIGHT_TASK_LIGHT_2_PWR_ON_SIGNAL      (1<<2)
#define  LIGHT_TASK_LIGHT_2_PWR_OFF_SIGNAL     (1<<3)
#define  LIGHT_TASK_ALL_SIGNALS                ((1<<4)-1)








#endif