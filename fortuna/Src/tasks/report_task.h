#ifndef  __DEVICE_TASK_H__
#define  __DEVICE_TASK_H__



#define  REPORT_TASK_INTERVAL          (1000*10)/*分钟*/             

#define  REPORT_TASK_RETRY_TIMEOUT     10/*重试的等待时间*/

/*设备上报任务*/
void   report_task(void const * argument);

extern osThreadId report_task_hdl;

#endif