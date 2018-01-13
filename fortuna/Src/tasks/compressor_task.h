#ifndef  __COMPRESSOR_TASK_H__
#define  __COMPRESSOR_TASK_H__


#define  COMPRESSOR_TASK_INTERVAL             1000/*压缩机任务间隔1秒*/

/*定义压缩机的温度工作区间*/
#define  COMPRESSOR_TASK_T_MIN                4
#define  COMPRESSOR_TASK_T_MAX                8
/*定义压缩机调试时工作时长*/
#define  COMPRESSOR_TASK_DEBUG_WORK_TIME      5000
/*压缩机连续工作的最大时间长 最长连续时间30分钟*/
#define  COMPRESSOR_TASK_WORK_MAX_TIME        (30*60*1000UL)
/*压缩机停止工作最小的时间长 最低需要暂停10分钟*/
#define  COMPRESSOR_TASK_STOP_MIN_TIME        (10*60*1000UL)

/*压缩机任务的信号*/
#define  COMPRESSOR_TASK_OPEN_SIGNAL           (1<<0)
#define  COMPRESSOR_TASK_CLOSE_SIGNAL          (1<<1)
#define  COMPRESSOR_TASK_ALL_SIGNALS           ((1<<2)-1)

#endif