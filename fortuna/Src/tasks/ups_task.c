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

static uint8_t ups_state;

uint8_t get_ups_state()
{
return ups_state;
}

static void update_ups_state()
{
/*UPS有两条信号指示IO 必须全部一致才可以判断UPS状态*/
bsp_state_t state1,state2;
state1=BSP_get_ups1_state();
state2=BSP_get_ups2_state();
/*如果UPS连接了主电源*/
if(state1==state2 && state2==UPS_PWR_STATE_ON)
ups_state=UPS_TASK_STATE_PWR_ON;
else
ups_state=UPS_TASK_STATE_PWR_OFF;
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