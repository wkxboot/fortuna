#ifndef  __TEMPERATURE_TASK_H__
#define  __TEMPERATURE_TASK_H__
#include "ntc_3950.h"

#define  TEMPERATURE_MONITOR_INTERVAL              50/*每隔50ms监视一次温度*/
#define  TEMPERATURE_SAMPLE_TIME                   500/*温度取样时间*/

#define  TEMPERATURE_CNT                           2/*2个温度计*/
int8_t get_temperature(uint8_t t_idx);





#endif