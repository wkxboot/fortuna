#ifndef  __DEVICE_STATUS_TASK_H__
#define  __DEVICE_STATUS_TASK_H__

/*设备状态任务 温度、门状态、UPS状态、按键检测等等*/
void device_status_task(void const * argument);
extern osThreadId device_status_task_hdl;
extern osMessageQId device_status_task_msg_q_id;

/*设备状态机*/
extern uint8_t ups_status,temperature;







#endif