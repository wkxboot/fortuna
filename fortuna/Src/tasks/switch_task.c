#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "display_led.h"
#include "display_task.h"
#include "switch_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[switch]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId switch_task_hdl;

typedef struct
{
 bsp_state_t pre_state;
 bsp_state_t cur_state;
 uint32_t    hold_on_time;/*此状态保持的时间*/
 uint32_t    short_press_time;/*短按有效时间*/
 uint32_t    long_press_time;/*长按有效时间*/
}switch_ctl_t;
 
void switch_task(void const * argument)
{
 APP_LOG_INFO("######按键状态任务开始.\r\n");
  /*从F-0依次显示，检测显示是否正常*/
 APP_LOG_DEBUG("初始化数码管显示...\r\n");
 for(uint8_t i=0x0f;i>0;i--)
 {
  for(uint8_t j=0;j<DISPLAY_LED_POS_CNT;j++)
  {
  dis_buff[i].num=i;  
  dis_buff[i].dp=FORTUNA_TRUE;
  }
  osDelay(INIT_DISPLAY_HOLD_ON_TIME);
 }
 APP_LOG_DEBUG("初始化数码管显示完毕.\r\n");
 while(1)
 {
 
 
 }
}