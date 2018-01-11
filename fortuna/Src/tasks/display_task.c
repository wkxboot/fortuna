#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "display_task.h"
#define APP_LOG_MODULE_NAME   "[display]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId display_task_hdl;
osMessageQId display_task_msg_q_id;

void display_task(void const * argument)
{
  APP_LOG_INFO("######显示任务开始.\r\n");
 while(1)
 {
  osDelay(10);
 }
  
  
  
}