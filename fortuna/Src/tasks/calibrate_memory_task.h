#ifndef  __CALIBRATE_MEMORY_TASK_H__
#define  __CALIBRATE_MEMORY_TASK_H__

/*校准显示缓存*/
extern dis_num_t calibrate_dis_buff[];

/*校准显示缓存任务*/
extern osThreadId calibrate_memory_task_hdl;
/*校准显示缓存任务*/
void calibrate_memory_task(void const * argument);



/*校准缓存任务运行间隔*/
#define  CALIBRATE_MEMORY_TASK_INTERVAL                  50

/*校准缓存任务信号*/
#define  CALIBRATE_MEMORY_TASK_CALIBRATE_START_SIGNAL    (1<<0)/**/
#define  CALIBRATE_MEMORY_TASK_UPDATE_POS_SIGNAL         (1<<1)
#define  CALIBRATE_MEMORY_TASK_INCREASE_SIGNAL           (1<<2)
#define  CALIBRATE_MEMORY_TASK_DECREASE_SIGNAL           (1<<3)
#define  CALIBRATE_MEMORY_TASK_CALIBRATE_OK_SIGNAL       (1<<4)
#define  CALIBRATE_MEMORY_TASK_CALIBRATE_ERR_SIGNAL      (1<<5)
#define  CALIBRATE_MEMORY_TASK_ALL_SIGNALS               ((1<<6)-1)

/*校准时相应位闪烁的时间*/
#define  CALIBRATE_MEMORY_TASK_FLASH_TIME                 400

/*外部使用*/
uint8_t get_calibrate_memory_calibrate_idx();
uint32_t get_calibrate_memory_calibrate_weight();



#endif