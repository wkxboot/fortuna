#ifndef  __TEMPERATURE_MEMORY_TASK_H__
#define  __TEMPERATURE_MEMORY_TASK_H__


/*温度显示缓存*/
extern dis_num_t t_dis_buff[DISPLAY_LED_POS_CNT];

/*温度显示缓存任务*/
extern osThreadId temperature_memory_task_hdl;
/*温度显示缓存任务*/
void temperature_memory_task(void const * argument);

/*温度显示缓存任务运行间隔*/
#define  TEMPERATURE_MEMORY_TASK_INTERVAL          1000


#define  TEMPERATURE_INVALID_VALUE                 99
#define  TEMPERATURE_INVALID_VALUE_NEGATIVE        (-9)


#endif