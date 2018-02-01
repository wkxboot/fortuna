#ifndef  __UPS_TASK_H__
#define  __UPS_TASK_H__


uint8_t get_ups_state();
/*UPS任务*/
void ups_task(void const * argument);
extern osThreadId ups_task_hdl;


/*UPS任务*/
#define  UPS_TASK_INTERVAL                         100/*UPS每隔100ms监视一次*/
/*UPS信号异常跳变过滤次数*/
#define  UPS_PWR_STATE_HOLD_CNT_MAX                5

#define  UPS_TASK_STATE_PWR_ON                     1
#define  UPS_TASK_STATE_PWR_OFF                    0
#define  UPS_TASK_STATE_ERR                        0xff

#endif