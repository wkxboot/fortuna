#ifndef  __DC_TASK_H__
#define  __DC_TASK_H__

/*电源任务*/
void dc_task(void const * argument);

extern osThreadId dc_task_hdl;

#define  DC_TASK_INTERVAL               osWaitForever
/*1任务信号*/
#define  DC_TASK_12V_PWR_ON_SIGNAL      (1<<0)
#define  DC_TASK_12V_PWR_OFF_SIGNAL     (1<<1)
#define  DC_TASK_24V_PWR_ON_SIGNAL      (1<<2)
#define  DC_TASK_24V_PWR_OFF_SIGNAL     (1<<3)
#define  DC_TASK_ALL_SIGNALS            ((1<<4)-1)






#endif 