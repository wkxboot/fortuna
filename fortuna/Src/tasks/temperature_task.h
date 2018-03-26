#ifndef  __TEMPERATURE_TASK_H__
#define  __TEMPERATURE_TASK_H__
#include "ntc_3950.h"

/*外部使用*/
int16_t get_temperature(uint8_t t_idx);
int16_t get_average_temperature();
uint8_t* get_average_temperature_str();
/*温度任务*/
void temperature_task(void const * argument);
extern osThreadId temperature_task_hdl;


/*一个变化方向上的保持时间最小值 10s*/
#define           T_HOLD_TIME                      10*1000

#define  TEMPERATURE_MONITOR_INTERVAL              50/*每隔50ms监视一次温度*/
#define  TEMPERATURE_SAMPLE_TIME                   1000/*温度取样时间*/
#define  TEMPERATURE_CNT                           1/*1个温度计*/

/*协议约定错误温度值*/
#define  TEMPERATURE_TASK_ERR_T_VALUE              0x7FFF
#define  TEMPERATURE_TASK_INVALID_VALUE            
#define  TEMPERATURE_TASK_INVALID_VALUE_NEGATIVE   (-99)



#endif