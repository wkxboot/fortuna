#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "digit_led.h"
#include "display_task.h"
#include "temperature_task.h"
#include "temperature_cache_task.h"
#define APP_LOG_MODULE_NAME   "[t_cache]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"

/*温度显示缓存*/
 dis_num_t t_dis_buff[DIGIT_LED_POS_CNT];

/*温度显示缓存任务*/
osThreadId temperature_cache_task_hdl;

void temperature_cache_task(void const * argument)
{
 int16_t t;
 APP_LOG_INFO("@温度显示缓存任务开始.\r\n");
 
 ptr_display_buff=t_dis_buff;
 while(1)
 {
 osDelay(TEMPERATURE_CACHE_TASK_INTERVAL);
 t=get_average_temperature();
 /*判断温度值范围 只有2位显示区域*/
 if(t==TEMPERATURE_TASK_ERR_T_VALUE || t > TEMPERATURE_CACHE_INVALID_VALUE ||t < TEMPERATURE_CAHCE_INVALID_VALUE_NEGATIVE)
 {
 /*超量程或者错误*/
 t_dis_buff[0].num=DIGIT_LED_NEGATIVE_NUM;
 t_dis_buff[0].dp=APP_FALSE;
 t_dis_buff[1].num=DIGIT_LED_NEGATIVE_NUM;
 t_dis_buff[1].dp=APP_FALSE;
 }
 else if(t>=0)/*正数范围*/
 {
 t_dis_buff[0].num=t/10;  
 t_dis_buff[0].dp=APP_TRUE;
 t_dis_buff[1].num=t%10;  
 t_dis_buff[1].dp=APP_FALSE;
 }
 else /*负数范围*/
 {
 t*=-1;
 t_dis_buff[0].num=DIGIT_LED_NEGATIVE_NUM;  
 t_dis_buff[0].dp=APP_FALSE;
 t_dis_buff[1].num=t%10;  
 t_dis_buff[1].dp=APP_FALSE;  
 }
 /*消0*/
 /* 小数只有一位暂时不需要消零
 if(t_dis_buff[0].num==0)
 t_dis_buff[0].num=DIGIT_LED_NULL_NUM;
*/
 }
}