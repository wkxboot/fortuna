#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "display_led.h"
#include "display_task.h"
#define APP_LOG_MODULE_NAME   "[display]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

#if (DISPLAY_TASK_HOLD_ON_TIME_PER_POS*DISPLAY_LED_POS_CNT*DISPLAY_TASK_FPS >=1000)
#error "显示参数设置错误，这些参数相乘应该小于1000ms"
#endif

osThreadId display_task_hdl;
osMessageQId display_task_msg_q_id;


/*显示任务的数字buff指针*/
dis_num_t *ptr_buff;



void display_task(void const * argument)
{
 uint32_t PreviousWakeTime;
 APP_LOG_INFO("######显示任务开始.\r\n");
 
 PreviousWakeTime = osKernelSysTick();
 while(1)
 {
  osDelayUntil(&PreviousWakeTime, DISPLAY_TASK_REFRESH_INTERVAL);
  if(ptr_buff==NULL)
    continue;/*还没有初始化 按键任务没有开始*/
  for(uint8_t i=0;i<DISPLAY_LED_POS_CNT;i++)
  {
  display_led_dis_num(1<<i,ptr_buff[i].num,ptr_buff[i].dp);
  osDelay(DISPLAY_TASK_HOLD_ON_TIME_PER_POS);
  }
 }
 
}