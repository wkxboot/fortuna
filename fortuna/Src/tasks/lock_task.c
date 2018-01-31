#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "lock_task.h"
#include "comm_protocol.h"
#include "ups_task.h"
#include "light_task.h"
#include "host_comm_task.h"
#include "glass_pwr_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[lock]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"



static uint8_t lock_task_update_lock_state();
static uint8_t lock_task_update_door_state();


/*任务和消息句柄*/
osThreadId lock_task_hdl;
osMessageQId lock_task_msg_q_id;
/*门锁状态机*/
uint8_t lock_state;
uint8_t door_state;

uint8_t get_lock_state()
{
  return lock_state;
}
uint8_t get_door_state()
{
  return door_state;
}
static uint8_t lock_task_update_lock_state()
{
  bsp_state_t state;
  state=BSP_get_lock_state();
  if(state==LOCK_STATE_OPEN)
    lock_state=LOCK_TASK_STATE_UNLOCKED ;
  else
    lock_state=LOCK_TASK_STATE_LOCKED;
  return lock_state;
}
static uint8_t lock_task_update_door_state()
{
  bsp_state_t state_up,state_dwn;
  state_up=BSP_get_door_up_state();
  state_dwn=BSP_get_door_dwn_state();
  if(state_up==state_dwn && state_up==LOCK_STATE_CLOSE)/*需要改为闭合判断*/
    door_state=LOCK_TASK_STATE_LOCKED ;
  else
    door_state=LOCK_TASK_STATE_UNLOCKED;
  return door_state; 
}

/*门锁任务*/
void lock_task(void const * argument)
{
 osEvent msg;
 lock_msg_t lock_msg;
 uint16_t time;
 
 APP_LOG_INFO("######门锁任务开始.\r\n");
 /*创建自己的消息队列*/
 osMessageQDef(lock_task_msg,2,uint32_t);
 lock_task_msg_q_id=osMessageCreate(osMessageQ(lock_task_msg),lock_task_hdl);
 APP_ASSERT(lock_task_msg_q_id);
 
 /*关闭门上灯*/
 BSP_LED_TURN_ON_OFF(DOOR_ORANGE_LED|DOOR_GREEN_LED|DOOR_RED_LED,LED_CTL_OFF); 
 /*打开GPU电源*/
 BSP_AC_TURN_ON_OFF(AC_1,AC_CTL_ON);
  /*打开交流风扇*/
 BSP_AC_TURN_ON_OFF(AC_2,AC_CTL_ON);
 while(1)
 {
  msg=osMessageGet(lock_task_msg_q_id,LOCK_TASK_INTERVAL);
  /*如果到达更新状态时间，那么就更新锁和门的状态*/
  if(msg.status == osEventTimeout)
  {
   lock_state=lock_task_update_lock_state();
   door_state=lock_task_update_door_state();
   /*如果UPS断电关闭玻璃、灯带电源*/
   if(get_ups_state()==UPS_TASK_STATE_PWR_OFF )
   {
    /*关闭灯带1和2*/
    if(BSP_get_light_state(LIGHT_1)==LIGHT_STATE_ON)
    osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_1_PWR_OFF_SIGNAL); 
    if(BSP_get_light_state(LIGHT_2)==LIGHT_STATE_ON)
    osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_2_PWR_OFF_SIGNAL); 
    /*关闭玻璃电源*/
    if(BSP_get_glass_pwr_state()==GLASS_PWR_STATE_ON)
    osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_OFF_SIGNAL);
   }
   else/*UPS有市电时*/
   {
    /*打开灯带1和2*/
    if(BSP_get_light_state(LIGHT_1)==LIGHT_STATE_OFF)
    osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_1_PWR_ON_SIGNAL); 
    if(BSP_get_light_state(LIGHT_2)==LIGHT_STATE_OFF)
    osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_2_PWR_ON_SIGNAL);  
    /*打开玻璃电源*/
    if(BSP_get_glass_pwr_state()==GLASS_PWR_STATE_OFF && lock_state==LOCK_TASK_STATE_LOCKED)
    osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_ON_SIGNAL);
   }
   continue;  
  }
  /*如果到不是锁消息，继续*/
  if(msg.status!=osEventMessage)
   continue;
  /*如果到是锁消息，执行对应的指令*/
  lock_msg=*(lock_msg_t*)&msg.value.v;
  switch(lock_msg.type)
  {
  case LOCK_TASK_UNLOCK_LOCK_MSG:
    APP_LOG_DEBUG("锁任务收到开锁指令消息.\r\n");
    time=0;
    while(time<LOCK_TASK_LOCK_TIMEOUT)
    {
    BSP_LOCK_TURN_ON_OFF(LOCK_CTL_OPEN);
    osDelay(LOCK_TASK_INTERVAL);
    lock_state=lock_task_update_lock_state();
    if(lock_state==LOCK_TASK_STATE_UNLOCKED)
      break;
    time+=LOCK_TASK_INTERVAL;
    }
    if(lock_state==LOCK_TASK_STATE_UNLOCKED)
    {
    /*关闭门上其余灯*/
    BSP_LED_TURN_ON_OFF(DOOR_ORANGE_LED,LED_CTL_OFF); 
    /*打开门上蓝色指示灯*/
    BSP_LED_TURN_ON_OFF(DOOR_GREEN_LED|DOOR_RED_LED,LED_CTL_ON);    
    /*关闭交流风扇*/
    BSP_AC_TURN_ON_OFF(AC_2,AC_CTL_OFF);
   /*不管UPS是否有市电都关闭玻璃加热电源*/
    APP_LOG_DEBUG("玻璃加热任务发送冷却信号.\r\n");
    osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_OFF_SIGNAL);
    APP_LOG_DEBUG("向通信任务发送开锁成功信号.\r\n");
    osSignalSet(host_comm_task_hdl,COMM_TASK_UNLOCK_LOCK_OK_SIGNAL); 
    }
    else
    {
    APP_LOG_ERROR("向通信任务发送开锁失败信号.\r\n");
    osSignalSet(host_comm_task_hdl,COMM_TASK_UNLOCK_LOCK_ERR_SIGNAL);  
    }      
    break;
  case LOCK_TASK_LOCK_LOCK_MSG:
    APP_LOG_DEBUG("锁任务收到关锁指令消息.\r\n");
    time=0;
    while(time<LOCK_TASK_LOCK_TIMEOUT)
    {
    BSP_LOCK_TURN_ON_OFF(LOCK_CTL_CLOSE);
    osDelay(LOCK_TASK_INTERVAL);
    lock_state=lock_task_update_lock_state();
    if(lock_state==LOCK_TASK_STATE_LOCKED)
      break;
    time+=LOCK_TASK_INTERVAL;
    }
    if(lock_state==LOCK_TASK_STATE_LOCKED)
    {
    /*关闭门上其余灯*/
    BSP_LED_TURN_ON_OFF(DOOR_GREEN_LED|DOOR_RED_LED,LED_CTL_OFF);
    /*打开门上橙色指示灯*/
    BSP_LED_TURN_ON_OFF(DOOR_ORANGE_LED,LED_CTL_ON);
    /*打开交流风扇*/
    BSP_AC_TURN_ON_OFF(AC_2,AC_CTL_ON);
    APP_LOG_DEBUG("向通信任务发送关锁成功信号.\r\n");
    osSignalSet(host_comm_task_hdl,COMM_TASK_LOCK_LOCK_OK_SIGNAL);
    if(get_ups_state()==UPS_TASK_STATE_PWR_ON)/*只有在UPS有市电的情况下才加热玻璃*/
    {
    APP_LOG_DEBUG("玻璃加热任务发送加热信号.\r\n");
    osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_ON_SIGNAL);
    }
    }
    else
    {
    APP_LOG_ERROR("向通信任务发送关锁失败信号.\r\n");
    osSignalSet(host_comm_task_hdl,COMM_TASK_LOCK_LOCK_ERR_SIGNAL);  
    }     
   break;
  default:
    APP_LOG_ERROR("错误的锁任务消息.%d\r\n",lock_msg.type);
  } 
 }
}