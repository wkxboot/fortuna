#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "dc12v_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[dc12v]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId dc12v_task_hdl;

void dc12v_task(void const * argument)
{
 osEvent signal;
 APP_LOG_INFO("######12V任务开始.\r\n");
 
 /*给自己发送打开12V电源信号*/
 osSignalSet(dc12v_task_hdl,DC12V_TASK_12V_1_PWR_ON_SIGNAL|DC12V_TASK_12V_2_PWR_ON_SIGNAL);
 
 while(1)
 {
 signal=osSignalWait(DC12V_TASK_ALL_SIGNALS,DC12V_TASK_INTERVAL);
 if(signal.status==osEventSignal)
 {
  if(signal.value.signals & DC12V_TASK_12V_1_PWR_ON_SIGNAL)
  {
   APP_LOG_DEBUG("12V任务打开输出1.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC12V_1,DC12V_CTL_ON);
  }
  if(signal.value.signals & DC12V_TASK_12V_1_PWR_OFF_SIGNAL)
  {
   APP_LOG_DEBUG("12V任务关闭输出1.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC12V_1,DC12V_CTL_OFF);
  }
  if(signal.value.signals & DC12V_TASK_12V_2_PWR_ON_SIGNAL)
  {
   APP_LOG_DEBUG("12V任务打开输出2.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC12V_2,DC12V_CTL_ON);
  }
  if(signal.value.signals & DC12V_TASK_12V_2_PWR_OFF_SIGNAL)
  {
   APP_LOG_DEBUG("12V任务关闭输出2.\r\n");
   BSP_DC12V_TURN_ON_OFF(DC12V_2,DC12V_CTL_OFF);
  }
 }
 }
}