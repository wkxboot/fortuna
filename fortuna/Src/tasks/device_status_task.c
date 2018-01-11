#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "device_status_task.h"
#include "iwdg.h"
#define APP_LOG_MODULE_NAME   "[status]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


osThreadId device_status_task_hdl;
osMessageQId device_status_task_msg_q_id;

uint8_t ups_status,temperature;

void device_status_task(void const * argument)
{
 APP_LOG_INFO("######设备状态任务开始.\r\n");
 while(1)
 {
 osDelay(200);
 /*喂狗*/
 sys_feed_dog();
}
  
  
}