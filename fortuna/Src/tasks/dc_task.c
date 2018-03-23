#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_AHG081_ZK.h"
#include "dc_task.h"
#define APP_LOG_MODULE_NAME   "[dc]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId dc_task_hdl;

void dc_task(void const * argument)
{
 osEvent signal;
 APP_LOG_INFO("@DC任务开始.\r\n");
 
 //osSignalSet(dc12v_task_hdl,DC12V_TASK_12V_1_PWR_ON_SIGNAL|DC12V_TASK_12V_2_PWR_ON_SIGNAL);
 
 while(1)
 {
 signal=osSignalWait(DC_TASK_ALL_SIGNALS,DC_TASK_INTERVAL);
 if(signal.status==osEventSignal)
 {
  if(signal.value.signals & DC_TASK_12V_PWR_ON_SIGNAL)
  {
   APP_LOG_DEBUG("DC任务打开12V输出.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC12V_CTL_ON);
  }
  if(signal.value.signals & DC_TASK_12V_PWR_OFF_SIGNAL)
  {
   APP_LOG_DEBUG("DC任务关闭12V输出.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC12V_CTL_OFF);
  }
  if(signal.value.signals & DC_TASK_24V_PWR_ON_SIGNAL)
  {
   APP_LOG_DEBUG("DC任务打开24V输出.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC24V_CTL_ON);
  }
  if(signal.value.signals & DC_TASK_24V_PWR_OFF_SIGNAL)
  {
   APP_LOG_DEBUG("DC任务关闭24V输出.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC24V_CTL_OFF);
  }
 }
 }
}