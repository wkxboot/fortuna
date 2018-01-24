#ifndef  __WEIGHT_MEMORY_TASK_H__
#define  __WEIGHT_MEMORY_TASK_H__


/*重量显示缓存任务*/
extern osThreadId weight_memory_task_hdl;
/*重量显示缓存任务*/
void weight_memory_task(void const * argument);

/*无效的重量值*/
#define  SCLAE_NET_WEIGHT_INVALID_VALUE                 99999
#define  SCLAE_NET_WEIGHT_INVALID_VALUE_NEGATIVE       (-9999)

/*重量显示缓存*/
extern dis_num_t w_dis_buff[];

/*重量显示缓存任务运行间隔*/
#define  WEIGHT_MEMORY_TASK_INTERVAL                   40

/*重量显示缓存任务信号*/
#define  WEIGHT_MEMORY_TASK_UPDATE_IDX_SIGNAL          (1<<0)
#define  WEIGHT_MEMORY_TASK_ALL_SIGNALS                ((1<<1)-1)


uint8_t get_weight_memory_idx();




#endif