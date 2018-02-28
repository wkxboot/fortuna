#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "display_led.h"
#include "display_task.h"
#include "ups_task.h"
#include "iwdg.h"
#include "light_task.h"
#include "lock_task.h"
#include "glass_pwr_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[ups]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"


osThreadId ups_task_hdl;
/*osMessageQId ups_task_msg_q_id;*/

static uint8_t ups_state=UPS_TASK_STATE_PWR_ON;

uint8_t get_ups_state()
{
return ups_state;
}

static void update_ups_state()
{
/*UPS有两条信号指示IO 必须全部一致才可以判断UPS状态*/
bsp_state_t state1,state2;
static uint8_t pwr_off_cnt=0;
static uint8_t pwr_on_cnt=0;
state1=BSP_get_ups1_state();
state2=BSP_get_ups2_state();
/*如果UPS连接了主电源*/
if(state1==UPS_PWR_STATE_ON && state2==UPS_PWR_STATE_ON)
{
 pwr_off_cnt=0;
 pwr_on_cnt++;
 if(pwr_on_cnt>=UPS_PWR_STATE_HOLD_CNT_MAX)/*过滤异常电平跳变*/
 {
 ups_state=UPS_TASK_STATE_PWR_ON;
 pwr_on_cnt=0;
 /*UPS有市电时*/
 /*打开灯带1和2*/
 if(BSP_get_light_state(LIGHT_1)==LIGHT_STATE_OFF)
 osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_1_PWR_ON_SIGNAL); 
 if(BSP_get_light_state(LIGHT_2)==LIGHT_STATE_OFF)
 osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_2_PWR_ON_SIGNAL);  
 /*打开玻璃电源*/
 if(BSP_get_glass_pwr_state()==GLASS_PWR_STATE_OFF && get_lock_state()==LOCK_TASK_STATE_LOCKED)
 osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_ON_SIGNAL);
 }
}
else
{
pwr_on_cnt=0;
pwr_off_cnt++;
if(pwr_off_cnt>=UPS_PWR_STATE_HOLD_CNT_MAX)/*过滤异常电平跳变*/
{
 ups_state=UPS_TASK_STATE_PWR_OFF;
 pwr_off_cnt=0;
  /*如果UPS断电关闭玻璃、灯带电源*/
  /*关闭灯带1和2*/
 if(BSP_get_light_state(LIGHT_1)==LIGHT_STATE_ON)
 osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_1_PWR_OFF_SIGNAL); 
 if(BSP_get_light_state(LIGHT_2)==LIGHT_STATE_ON)
 osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_2_PWR_OFF_SIGNAL); 
 /*关闭玻璃电源*/
 if(BSP_get_glass_pwr_state()==GLASS_PWR_STATE_ON)
 osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_OFF_SIGNAL);
}
}
}

/*UPS状态查询任务*/
void ups_task(void const * argument)
{
 APP_LOG_INFO("######UPS状态任务开始.\r\n");
 while(1)
 {
 osDelay(UPS_TASK_INTERVAL);
 update_ups_state();
 }
   
}