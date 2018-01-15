#ifndef  __TEMPERATURE_TASK_H__
#define  __TEMPERATURE_TASK_H__
#include "ntc_3950.h"

/*外部使用*/
int8_t get_temperature(uint8_t t_idx);
int8_t get_average_temperature();
/*温度任务*/
void temperature_task(void const * argument);
extern osThreadId temperature_task_hdl;

#define  TEMPERATURE_MONITOR_INTERVAL              50/*每隔50ms监视一次温度*/
#define  TEMPERATURE_SAMPLE_TIME                   500/*温度取样时间*/

#define  TEMPERATURE_CNT                           2/*2个温度计*/

/*协议约定错误温度值*/
#define  TEMPERATURE_TASK_ERR_T_VALUE              0x7f




#endif