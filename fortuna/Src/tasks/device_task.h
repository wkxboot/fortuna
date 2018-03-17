#ifndef  __DEVICE_TASK_H__
#define  __DEVICE_TASK_H__



#define  DEVICE_TASK_INTERVAL          (1000*10)/*分钟*/             



/*设备上报任务*/
void device_report_task(void const * argument);

extern osThreadId device_report_task_hdl;

#endif