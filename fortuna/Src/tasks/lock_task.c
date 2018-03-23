#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_AHG081_ZK.h"
#include "lock_task.h"
#include "ups_task.h"
#include "light_task.h"
#include "glass_pwr_task.h"
#include "fan_task.h"
#include "comm_protocol.h"
#include "host_comm_task.h"
#define APP_LOG_MODULE_NAME   "[lock]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


/*任务和消息句柄*/
osThreadId lock_task_hdl;

osTimerId lock_timer_id;
osTimerId unlock_timer_id;


static void lock_timer_init();
static void lock_timer_start();
static void lock_timer_stop();
static void lock_timer_expired(void const * argument);

static void unlock_timer_init();
static void unlock_timer_start();
static void unlock_timer_stop();
static void unlock_timer_expired(void const * argument);

static void lock_task_turn_on_lock_led();
static void lock_task_turn_on_unlock_led();

static void lock_task_lock_lock();
static void lock_task_unlock_lock();

/*调试调用*/
static void lock_task_debug_lock_lock();
static void lock_task_debug_unlock_lock();

static uint8_t lock_status=LOCK_FAULT;


static void lock_timer_init()
{
 osTimerDef(lock_timer,lock_timer_expired);
 lock_timer_id=osTimerCreate(osTimer(lock_timer),osTimerOnce,0);
 APP_ASSERT(lock_timer_id);
}

static void lock_timer_start()
{
 APP_LOG_DEBUG("关锁定时器开始.\r\n");
 osTimerStart(lock_timer_id,LOCK_TASK_LOCK_TIMER_TIMEOUT);
}
static void lock_timer_stop()
{
 APP_LOG_DEBUG("关锁定时器停止.\r\n");
 osTimerStop(lock_timer_id);
}

static void lock_timer_expired(void const * argument)
{
 APP_LOG_DEBUG("关锁定时器到达.\r\n");
 if(bsp_get_lock_status()==LOCK_STATUS_LOCK)
 {
  APP_LOG_DEBUG("向主机通信任务发送关锁成功信号.\r\n");
  osSignalSet(host_comm_task_hdl,COMM_TASK_LOCK_LOCK_SUCCESS_SIGNAL);
 
  lock_task_turn_on_lock_led();
 }
 else
 {
  APP_LOG_DEBUG("向主机通信任务发送关锁失败信号.\r\n");
  osSignalSet(host_comm_task_hdl,COMM_TASK_LOCK_LOCK_FAIL_SIGNAL); 
  
  lock_task_unlock_lock();
 }
}

static void unlock_timer_init()
{
 osTimerDef(unlock_timer,unlock_timer_expired);
 unlock_timer_id=osTimerCreate(osTimer(unlock_timer),osTimerOnce,0);
 APP_ASSERT(unlock_timer_id);
}
static void unlock_timer_start()
{
 APP_LOG_DEBUG("开锁定时器开始.\r\n");
 osTimerStart(unlock_timer_id,LOCK_TASK_LOCK_TIMER_TIMEOUT);
}
static void unlock_timer_stop()
{
 APP_LOG_DEBUG("开锁定时器停止.\r\n");
 osTimerStop(unlock_timer_id);
}

static void unlock_timer_expired(void const * argument)
{
 APP_LOG_DEBUG("开锁定时器到达.\r\n");
 if(bsp_get_lock_status()==LOCK_STATUS_UNLOCK)
 {
  APP_LOG_DEBUG("向主机通信任务发送开锁成功信号.\r\n");
  osSignalSet(host_comm_task_hdl,COMM_TASK_UNLOCK_LOCK_SUCCESS_SIGNAL);

  lock_task_turn_on_unlock_led();
 }
 else
 {
  APP_LOG_DEBUG("向主机通信任务发送开锁失败信号.\r\n");
  osSignalSet(host_comm_task_hdl,COMM_TASK_UNLOCK_LOCK_FAIL_SIGNAL); 
  
  /*如果开锁失败就立刻把锁锁死*/
  lock_task_lock_lock();

 }
}

static void lock_task_lock_lock()
{
 BSP_LOCK_CTL(LOCK_CTL_LOCK); 
}
static void lock_task_unlock_lock()
{
 BSP_LOCK_CTL(LOCK_CTL_UNLOCK); 
}

/*关锁成功时 打开橙色led*/
static void lock_task_turn_on_lock_led()
{
 APP_LOG_DEBUG("打开绿色门灯.\r\n");
 BSP_LED_TURN_ON_OFF(DOOR_GREEN_LED,LED_CTL_ON); 
 BSP_LED_TURN_ON_OFF(DOOR_ORANGE_LED,LED_CTL_OFF);
}
/*开锁成功时 打开绿色led*/
static void lock_task_turn_on_unlock_led()
{
 APP_LOG_DEBUG("打开橙色门灯.\r\n");
 BSP_LED_TURN_ON_OFF(DOOR_ORANGE_LED,LED_CTL_ON); 
 BSP_LED_TURN_ON_OFF(DOOR_GREEN_LED,LED_CTL_OFF); 
}

/*调试调用*/
static void lock_task_debug_lock_lock()
{
  BSP_LOCK_CTL(LOCK_CTL_LOCK);  
}
/*调试调用*/
static void lock_task_debug_unlock_lock()
{
  BSP_LOCK_CTL(LOCK_CTL_UNLOCK);  
}

/*锁任务*/
void lock_task(void const * argument)
{
  osEvent sig;
  APP_LOG_INFO("@锁任务开始.\r\n");
  
  lock_timer_init();
  unlock_timer_init();
  
  /*首先在开机时判断锁的状态*/
  if(bsp_get_lock_status()==LOCK_STATUS_UNLOCK)
  lock_task_turn_on_unlock_led();
  else
  lock_task_turn_on_lock_led();
  
  
  while(1)
  {
  sig=osSignalWait(LOCK_TASK_ALL_SIGNALS,LOCK_TASK_WAIT_TIMEOUT);
  if(sig.status==osEventSignal)
  {
   /*收到关锁信号*/
   if(sig.value.signals & LOCK_TASK_LOCK_SIGNAL)
   {
    APP_LOG_DEBUG("收到关锁信号.关锁.\r\n");
    unlock_timer_stop();
    lock_timer_start();
    lock_task_lock_lock();
   }
   /*收到开锁信号*/
   if(sig.value.signals & LOCK_TASK_UNLOCK_SIGNAL)
   {
    APP_LOG_DEBUG("收到开锁信号.开锁.\r\n");
    lock_timer_stop();
    unlock_timer_start();
    lock_task_unlock_lock();
   }
   /*收到门的状态变为打开*/
   if(sig.value.signals & LOCK_TASK_DOOR_STATUS_OPEN_SIGNAL)
   {
    APP_LOG_DEBUG("门状态变化->打开.\r\n");
    /*避免死锁 重复开锁一次*/
    lock_task_unlock_lock();
    /*门打开后关闭风扇和加热玻璃*/
    osSignalSet(fan_task_hdl,FAN_TASK_FAN_TURN_OFF_SIGNAL);
    osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_TURN_OFF_SIGNAL);
   }
   /*收到门的状态变为关闭*/
   if(sig.value.signals & LOCK_TASK_DOOR_STATUS_CLOSE_SIGNAL)
   {
    APP_LOG_DEBUG("门状态变化->关闭.关锁.\r\n");
    unlock_timer_stop();
    lock_timer_start();
    /*马上尝试关锁*/
    lock_task_lock_lock();
   /*门关闭后打开风扇和加热玻璃*/
    osSignalSet(fan_task_hdl,FAN_TASK_FAN_TURN_ON_SIGNAL);
    osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_TURN_ON_SIGNAL);
   }   
   /*收到调试开锁信号 打开*/
   if(sig.value.signals & LOCK_TASK_DEBUG_UNLOCK_SIGNAL)
   {
    lock_task_debug_unlock_lock();
    APP_LOG_DEBUG("调试开锁.\r\n");
   }
   /*收到调试关锁信号 关锁*/
   if(sig.value.signals & LOCK_TASK_DEBUG_LOCK_SIGNAL)
   {
    APP_LOG_DEBUG("调试关锁.\r\n");
    lock_task_debug_lock_lock();
   }     
  }
 }
}
 
 
 