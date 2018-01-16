#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "scales.h"
#include "display_led.h"
#include "display_task.h"
#include "switch_task.h"
#include "temperature_task.h"
#include "temperature_memory_task.h"
#define APP_LOG_MODULE_NAME   "[t_mem]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

/*温度显示缓存*/
 dis_num_t t_dis_buff[DISPLAY_LED_POS_CNT];

/*温度显示缓存任务*/
osThreadId temperature_memory_task_hdl;

void temperature_memory_task(void const * argument)
{
 static int8_t t;
 APP_LOG_INFO("######温度显示缓存任务开始.\r\n");
 while(1)
 {
 osDelay(TEMPERATURE_MEMORY_TASK_INTERVAL);
 t=get_average_temperature();
 if(t<0)
 {
 t_dis_buff[0].num=DISPLAY_LED_NEGATIVE_NUM;
 t*=-1;/*需要变换成正数*/
 }
 else
 t_dis_buff[0].num=DISPLAY_LED_NULL_NUM;  
 
 t_dis_buff[1].num=DISPLAY_LED_NULL_NUM;
 t_dis_buff[2].num=DISPLAY_LED_NULL_NUM;
 t_dis_buff[3].num=DISPLAY_LED_NULL_NUM;
 t_dis_buff[4].num=t/10;
 t_dis_buff[5].num=t%10;
 /*消0*/
 if(t_dis_buff[4].num==0)
 t_dis_buff[4].num=DISPLAY_LED_NULL_NUM;
 }
}