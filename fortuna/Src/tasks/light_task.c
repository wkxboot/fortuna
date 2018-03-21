#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "light_task.h"

#define APP_LOG_MODULE_NAME   "[light]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId light_task_hdl;
app_bool_t is_light_turn_on_enable=APP_TRUE;

static void light_task_light_turn_on()
{
 if(is_light_turn_on_enable==APP_TRUE)
 {
  APP_LOG_DEBUG("打开灯条.\r\n");
  BSP_LIGHT_TURN_ON_OFF(LIGHT_CTL_ON);
 }
 else
 {
  APP_LOG_WARNING("UPS断开市电无法打开灯条.\r\n");  
 }
}
static void light_task_light_turn_off()
{
 APP_LOG_DEBUG("关闭灯条.\r\n");
 BSP_LIGHT_TURN_ON_OFF(LIGHT_CTL_OFF);
}

static void light_task_debug_light_turn_on()
{
 APP_LOG_DEBUG("调试打开灯条.\r\n");
 BSP_LIGHT_TURN_ON_OFF(LIGHT_CTL_ON); 
}
static void light_task_debug_light_turn_off()
{
 APP_LOG_DEBUG("调试关闭灯条.\r\n");
 BSP_LIGHT_TURN_ON_OFF(LIGHT_CTL_OFF); 
}



void light_task(void const * argument)
{
 osEvent signal;
 APP_LOG_INFO("@灯条任务开始.\r\n");
 /*首先关闭灯条*/
 light_task_light_turn_off();
 
 while(1)
 {
 signal=osSignalWait(LIGHT_TASK_ALL_SIGNALS,LIGHT_TASK_INTERVAL);
 if(signal.status==osEventSignal)
 {
 if(signal.value.signals & LIGHT_TASK_LIGHT_TURN_ON_SIGNAL)
 { 
   if(is_light_turn_on_enable==APP_TRUE)
   light_task_light_turn_on();
 }
 if(signal.value.signals & LIGHT_TASK_LIGHT_TURN_OFF_SIGNAL)
 {
  light_task_light_turn_off();
 }
 if(signal.value.signals & LIGHT_TASK_DEBUG_LIGHT_TURN_ON_SIGNAL)
 {
  light_task_debug_light_turn_on();
 }
 if(signal.value.signals & LIGHT_TASK_DEBUG_LIGHT_TURN_OFF_SIGNAL)
 {
  light_task_debug_light_turn_off();
 }
 /*根据UPS状态设置灯光可控使能*/
 if(signal.value.signals & LIGHT_TASK_UPS_PWR_ON_SIGNAL)
 {
  APP_LOG_DEBUG("LIGHT任务收到UPS变化.\r\n");
  is_light_turn_on_enable=APP_TRUE;
  light_task_light_turn_on();
 }
 if(signal.value.signals & LIGHT_TASK_UPS_PWR_OFF_SIGNAL)
 {
  APP_LOG_DEBUG("LIGHT任务收到UPS变化.\r\n");
  is_light_turn_on_enable=APP_FALSE;
  light_task_light_turn_off();
 }
 
 
 
 
 }
 } 
}