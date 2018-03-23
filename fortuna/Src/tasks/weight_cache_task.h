#ifndef  __WEIGHT_CACHE_TASK_H__
#define  __WEIGHT_CACHE_TASK_H__


/*重量显示缓存任务*/
extern osThreadId weight_cache_task_hdl;
/*重量显示缓存任务*/
void weight_cache_task(void const * argument);

/*重量显示缓存*/
extern dis_num_t w_dis_buff[];

/*重量显示缓存任务运行间隔*/
#define  WEIGHT_CACHE_TASK_INTERVAL                   40

/*重量显示缓存任务信号*/
#define  WEIGHT_CACHE_TASK_UPDATE_IDX_SIGNAL          (1<<0)
#define  WEIGHT_CACHE_TASK_ALL_SIGNALS                ((1<<1)-1)


uint8_t get_weight_cache_idx();




#endif