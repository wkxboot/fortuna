#ifndef  __DC12V_TASK_H__
#define  __DC12V_TASK_H__

/*12V电源任务*/
void dc12v_task(void const * argument);

extern osThreadId dc12v_task_hdl;

#define  DC12V_TASK_INTERVAL                 osWaitForever
/*12V任务信号*/
#define  DC12V_TASK_12V_1_PWR_ON_SIGNAL      (1<<0)
#define  DC12V_TASK_12V_1_PWR_OFF_SIGNAL     (1<<1)
#define  DC12V_TASK_12V_2_PWR_ON_SIGNAL      (1<<2)
#define  DC12V_TASK_12V_2_PWR_OFF_SIGNAL     (1<<3)
#define  DC12V_TASK_ALL_SIGNALS              ((1<<4)-1)






#endif 