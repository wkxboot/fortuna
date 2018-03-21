#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "sys_led_task.h"
#include "ABDK_ABX081_ZK.h"
#define APP_LOG_MODULE_NAME   "[sys_led]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId sys_led_task_hdl;

void sys_led_task(void const * argument)
{
 APP_LOG_INFO("@系统运行灯任务开始.\r\n");
 while(1)
 {
  BSP_LED_TURN_ON_OFF(SYS_LED,LED_CTL_ON);
  osDelay(SYS_LED_TASK_INTERVAL);
  BSP_LED_TURN_ON_OFF(SYS_LED,LED_CTL_OFF);
  osDelay(SYS_LED_TASK_INTERVAL);
 }
}