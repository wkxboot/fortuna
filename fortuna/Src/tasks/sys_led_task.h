#ifndef  __SYS_LED_TASK_H__
#define  __SYS_LED_TASK_H__

extern osThreadId sys_led_task_hdl;
/*系统运行灯任务*/
void sys_led_task(void const * argument);




/*系统运行灯任务间隔*/
#define  SYS_LED_TASK_INTERVAL           400



#endif