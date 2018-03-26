#ifndef  __TEMPERATURE_MEMORY_TASK_H__
#define  __TEMPERATURE_MEMORY_TASK_H__


/*温度显示缓存*/
extern dis_num_t t_dis_buff[];

/*温度显示缓存任务*/
extern osThreadId temperature_cache_task_hdl;
/*温度显示缓存任务*/
void temperature_cache_task(void const * argument);

/*温度显示缓存任务运行间隔*/
#define  TEMPERATURE_CACHE_TASK_INTERVAL                 1000


#define  TEMPERATURE_CACHE_INVALID_VALUE                  99
#define  TEMPERATURE_CAHCE_INVALID_VALUE_NEGATIVE        (-99)


#endif