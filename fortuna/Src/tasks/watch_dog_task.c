#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "iwdg.h"
#include "watch_dog_task.h"
#define APP_LOG_MODULE_NAME   "[watch_dog]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"

osThreadId watch_dog_task_hdl;
void watch_dog_task(void const * argument)
{
 APP_LOG_INFO("@看门狗任务开始.\r\n");
 while(1)
 {
 osDelay(WATCH_DOG_TASK_INTERVAL);
 /*喂狗*/
 APP_LOG_DEBUG("喂狗.\r\n");
 sys_feed_dog();
 }
}