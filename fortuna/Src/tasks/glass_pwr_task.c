#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "glass_pwr_task.h"

#define APP_LOG_MODULE_NAME   "[galss_pwr]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

/*加热玻璃任务*/
osThreadId glass_pwr_task_hdl;
osTimerId glass_pwr_timer_id;


static void glass_pwr_timer_init();
static void glass_pwr_timer_start();
static void glass_pwr_timer_stop();
static void glass_pwr_timer_expired(void const * argument);

app_bool_t is_glass_pwr_turn_on_enable=APP_TRUE;

static void glass_pwr_task_turn_on()
{
 if(is_glass_pwr_turn_on_enable==APP_TRUE)
 {
  APP_LOG_DEBUG("打开加热玻璃.\r\n");
  BSP_GLASS_PWR_TURN_ON_OFF(LIGHT_CTL_ON);
  glass_pwr_timer_start();
 }
 else
 {
  APP_LOG_WARNING("UPS断开市电无法打开加热玻璃.\r\n");  
 }
}
static void glass_pwr_task_turn_off()
{
 APP_LOG_DEBUG("关闭加热玻璃.\r\n");
 BSP_GLASS_PWR_TURN_ON_OFF(LIGHT_CTL_OFF);
 glass_pwr_timer_stop();
}

static void glass_pwr_task_debug_turn_on()
{
 APP_LOG_DEBUG("调试打开加热玻璃.\r\n");
 BSP_GLASS_PWR_TURN_ON_OFF(LIGHT_CTL_ON); 
}
static void glass_pwr_task_debug_turn_off()
{
 APP_LOG_DEBUG("调试打开加热玻璃.\r\n");
 BSP_GLASS_PWR_TURN_ON_OFF(LIGHT_CTL_OFF); 
}



static void glass_pwr_timer_init()
{
 osTimerDef(glass_pwr_timer,glass_pwr_timer_expired);
 glass_pwr_timer_id=osTimerCreate(osTimer(glass_pwr_timer),osTimerOnce,0);
 APP_ASSERT(glass_pwr_timer_id);
}

static void glass_pwr_timer_start()
{
 APP_LOG_DEBUG("关锁定时器开始.\r\n");
 osTimerStart(glass_pwr_timer_id,GLASS_PWR_TASK_WORK_TIME_MAX);
}
static void glass_pwr_timer_stop()
{
 APP_LOG_DEBUG("关锁定时器停止.\r\n");
 osTimerStop(glass_pwr_timer_id);
}

static void glass_pwr_timer_expired(void const * argument)
{
 glass_pwr_task_turn_off();
}

void glass_pwr_task(void const * argument)
{
 osEvent signal;
 /*加热玻璃状态*/

 APP_LOG_INFO("@玻璃温度控制任务开始.\r\n");
 
 glass_pwr_timer_init();
 glass_pwr_task_turn_off();
 
 while(1)
 {
  signal=osSignalWait(GLASS_PWR_TASK_ALL_SIGNALS,GLASS_PWR_TASK_INTERVAL);
  if(signal.status==osEventSignal)
  {
   if(signal.value.signals & GLASS_PWR_TASK_TURN_ON_SIGNAL)
   {
    APP_LOG_DEBUG("收到加热玻璃打开信号.\r\n");
    glass_pwr_task_turn_on();
   }
   if(signal.value.signals & GLASS_PWR_TASK_TURN_OFF_SIGNAL)
   {
    APP_LOG_DEBUG("收到加热玻璃关闭信号.\r\n");
    glass_pwr_task_turn_off();
   }
   if(signal.value.signals & GLASS_PWR_TASK_DEBUG_TURN_ON_SIGNAL)
   {
    APP_LOG_DEBUG("收到调试加热玻璃打开信号.\r\n");
    glass_pwr_task_debug_turn_on();
   }
   if(signal.value.signals & GLASS_PWR_TASK_DEBUG_TURN_OFF_SIGNAL)
   {
    APP_LOG_DEBUG("收到调试加热玻璃关闭信号.\r\n");
    glass_pwr_task_debug_turn_off();
   }
   if(signal.value.signals & GLASS_PWR_TASK_UPS_PWR_ON_SIGNAL)
   {
    APP_LOG_DEBUG("加热玻璃收到UPS接入市电信号.\r\n");
    is_glass_pwr_turn_on_enable=APP_TRUE;
   }
   if(signal.value.signals & GLASS_PWR_TASK_UPS_PWR_OFF_SIGNAL)
   {
    APP_LOG_DEBUG("加热玻璃收到UPS断开市电信号.\r\n");
    is_glass_pwr_turn_on_enable=APP_FALSE;
    glass_pwr_task_turn_off();
   }   
  }
  
 }
} 
