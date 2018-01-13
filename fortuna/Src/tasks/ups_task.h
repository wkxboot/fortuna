#ifndef  __UPS_TASK_H__
#define  __UPS_TASK_H__

/*UPS状态任务*/
void device_status_task(void const * argument);
extern osThreadId device_status_task_hdl;
extern osMessageQId device_status_task_msg_q_id;

/*设备状态机*/
extern uint8_t ups_status,temperature;

#define  UPS_TASK_INTERVAL                         100/*UPS每隔100ms监视一次*/

#define  UPS_TASK_STATE_PWR_ON                     1
#define  UPS_TASK_STATE_PWR_OFF                    0
#define  UPS_TASK_STATE_ERR                        0xff

#endif