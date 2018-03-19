#ifndef  __GLASS_PWR_TASK_H__
#define  __GLASS_PWR_TASK_H__

/*玻璃加热电源任务*/
void glass_pwr_task(void const * argument);
extern osThreadId glass_pwr_task_hdl;


/*玻璃加热任务运行间隔.不需要实时响应.所以间隔大些*/
#define  GLASS_PWR_TASK_INTERVAL                   50
/*玻璃加热任务工作最大时长 5分钟*/
#define  GLASS_PWR_TASK_WORK_TIME_MAX              (5*60*1000UL)
/*玻璃加热任务信号*/
#define  GLASS_PWR_TASK_TURN_ON_SIGNAL             (1<<0)
#define  GLASS_PWR_TASK_TURN_OFF_SIGNAL            (1<<1)
#define  GLASS_PWR_TASK_DEBUG_TURN_ON_SIGNAL       (1<<2)
#define  GLASS_PWR_TASK_DEBUG_TURN_OFF_SIGNAL      (1<<3)
#define  GLASS_PWR_TASK_UPS_PWR_ON_SIGNAL          (1<<4)
#define  GLASS_PWR_TASK_UPS_PWR_OFF_SIGNAL         (1<<5)
#define  GLASS_PWR_TASK_ALL_SIGNALS                ((1<<6)-1)

#endif