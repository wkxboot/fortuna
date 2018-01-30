#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "glass_pwr_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[galss_pwr]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

/*加热玻璃任务*/
osThreadId glass_pwr_task_hdl;

void glass_pwr_task(void const * argument)
{
 osEvent signal;
 uint32_t pre_time,cur_time,work_time=0;
 /*加热玻璃状态*/
 bsp_state_t glass_pwr_state;
 APP_LOG_INFO("######玻璃温度控制任务开始.\r\n");
 /*首先关闭玻璃加热*/
 osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_OFF_SIGNAL);
 /*计算加热时间*/
 pre_time=osKernelSysTick();
 cur_time=pre_time;
 while(1)
 {
  signal=osSignalWait(GLASS_PWR_TASK_ALL_SIGNALS,GLASS_PWR_TASK_INTERVAL);
  cur_time=osKernelSysTick();
  glass_pwr_state=BSP_get_glass_pwr_state();
  /*更新工作时长*/
  if(glass_pwr_state==GLASS_PWR_STATE_ON)
  {
  work_time+=cur_time-pre_time;
  }
  pre_time=cur_time;
  
  if(signal.status==osEventSignal)
  {
   if(signal.value.signals & GLASS_PWR_TASK_ON_SIGNAL)
   {
    APP_LOG_DEBUG("玻璃温度控制任务收到加热信号.\r\n");  
    if(glass_pwr_state!=GLASS_PWR_STATE_ON)
    {
    APP_LOG_DEBUG("玻璃温度控制任务加热玻璃.\r\n");  
    BSP_GLASS_PWR_TURN_ON_OFF(GLASS_PWR_CTL_ON); 
    work_time=0;
    }
   }
   if(signal.value.signals & GLASS_PWR_TASK_OFF_SIGNAL)
   {
    APP_LOG_DEBUG("玻璃温度控制任务收到关闭加热信号.\r\n");
    if(glass_pwr_state!=GLASS_PWR_STATE_OFF)
    {
    APP_LOG_DEBUG("玻璃温度控制任务关闭加热玻璃.\r\n");  
    BSP_GLASS_PWR_TURN_ON_OFF(GLASS_PWR_CTL_OFF); 
    work_time=0;
    }
   }
  }
  
  if(work_time>=GLASS_PWR_TASK_WORK_TIME_MAX)
  {
   APP_LOG_DEBUG("玻璃加热到达时间.发送关闭加热信号.\r\n");
   osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_OFF_SIGNAL); 
  }
  
 }
} 
