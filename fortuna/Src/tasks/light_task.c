#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "light_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[light]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId light_task_hdl;

void light_task(void const * argument)
{
 osEvent signal;
 APP_LOG_INFO("######灯条任务开始.\r\n");
 /*由UPS状态操作灯条*/
 /*关闭暂时定义自己打开*/
 /*
 osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_1_PWR_ON_SIGNAL|LIGHT_TASK_LIGHT_2_PWR_ON_SIGNAL);
 */
 while(1)
 {
 signal=osSignalWait(LIGHT_TASK_ALL_SIGNALS,LIGHT_TASK_INTERVAL);
 if(signal.status==osEventSignal)
 {
 if(signal.value.signals & LIGHT_TASK_LIGHT_1_PWR_ON_SIGNAL)
 { 
  BSP_LIGHT_TURN_ON_OFF(LIGHT_1,LIGHT_CTL_ON);
  APP_LOG_DEBUG("打开灯条1.\r\n");
 }
 if(signal.value.signals & LIGHT_TASK_LIGHT_1_PWR_OFF_SIGNAL)
 {
  BSP_LIGHT_TURN_ON_OFF(LIGHT_1,LIGHT_CTL_OFF);
  APP_LOG_DEBUG("关闭灯条1.\r\n");
 }
 if(signal.value.signals & LIGHT_TASK_LIGHT_2_PWR_ON_SIGNAL)
 {
  BSP_LIGHT_TURN_ON_OFF(LIGHT_2,LIGHT_CTL_ON);
  APP_LOG_DEBUG("打开灯条2.\r\n");
 }
 if(signal.value.signals & LIGHT_TASK_LIGHT_2_PWR_OFF_SIGNAL)
 {
  BSP_LIGHT_TURN_ON_OFF(LIGHT_2,LIGHT_CTL_OFF);
  APP_LOG_DEBUG("关闭灯条2.\r\n");
 }
 }
 } 
}