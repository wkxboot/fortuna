#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "string.h"
#include "fortuna_common.h"
#include "scales.h"
#include "host_protocol.h"
#include "host_comm_task.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#include "scale_func_task.h"
#include "lock_task.h"
#include "debug_task.h"
#define APP_LOG_MODULE_NAME   "[debug]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

#define  DEBUG_CMD_MAX_LEN    20

uint8_t cmd[DEBUG_CMD_MAX_LEN];
/*RTT调试任务*/
void debug_task(void const * argument)
{
 uint8_t debug_enable=FORTUNA_FALSE;
 scale_msg_t scale_msg;
 uint16_t data_cnt;
 APP_LOG_INFO("######调试任务开始.\r\n");

 while(1)
 {
  osDelay(100); 
  data_cnt=SEGGER_RTT_HasData(0);
  /*buff0没有数据*/
  if(data_cnt==0)
    continue;  
  data_cnt=SEGGER_RTT_Read(0,cmd,data_cnt);
  if(debug_enable!=FORTUNA_TRUE)
    continue;
  APP_LOG_DEBUG("读的字节数：%d\r\n",data_cnt);  
  if(data_cnt>DEBUG_CMD_MAX_LEN-1)
    data_cnt=DEBUG_CMD_MAX_LEN-1;
  cmd[data_cnt]=0;/*填充为完整字符串*/
  /*设置地址*/
  uint8_t time,origin_addr,now_addr,len;
  if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_ADDR,sizeof(DEBUG_TASK_CMD_SET_ADDR))==0)
  {
  len=sizeof(DEBUG_TASK_CMD_SET_ADDR);
  origin_addr=cmd[len];
  now_addr=cmd[len+1];
  if(origin_addr > '9' || origin_addr <'1' ||now_addr > '9' || now_addr <'0' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_SET_ADDR)+2)
  {
   APP_LOG_ERROR("命令长度或者地址值非法.地址范围1-9.\r\n");
   continue;
  }
  origin_addr-='0';
  now_addr-='0';
  APP_LOG_DEBUG("原地址：%2d设置为%2d.\r\n",origin_addr,now_addr); 
  scale_msg.type=SCALE_FUNC_TASK_SET_ADDR_MSG;
  scale_msg.scale=origin_addr;
  scale_msg.param16=now_addr;
  time=0;
  while(time < DEBUG_TASK_CMD_SET_ADDR_TIMEOUT)
  {
  APP_LOG_DEBUG("向电子秤功能任务发送设置地址消息.\r\n");
  osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
  signal=osSignalWait(DEBUG_TASK_SET_ADDR_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
  if(signal.status!=osEventSignal || !(signal.value.signals & DEBUG_TASK_SET_ADDR_OK_SIGNAL))/*超时或者其他错误*/
  break;
  time+=DEBUG_TASK_WAIT_TIMEOUT;
  }
 if(time > DEBUG_TASK_SET_ADDR_TIMEOUT)
 {
 }
 }

  /*获取净重值*/
  uint16_t weight;
  uint16_t time;
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT,sizeof(DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT))==0)
 {
 len=sizeof(DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT);
 origin_addr=cmd[len];
 if(origin_addr > '9' || origin_addr <'1')
 {
  APP_LOG_ERROR("设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤%2d获取净重...",origin_addr);
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG;
 scale_msg.scale=origin_addr;
 /*向电子秤功能任务发送获取净重消息*/
 while(time<DEBUG_TASK_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status!=osEventSignal || !(signal.value.signals & DEBUG_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL))/*超时或者其他错误*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time > DEBUG_TASK_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤解锁超时.");
 }
 else
 {
 APP_LOG_INFO("电子秤设备解锁成功.");
 }
 continue;
  
  
  }
 /*设备解锁*/
 if(strcmp((const char*)cmd,DEBUG_TASK_CMD_DEVICE_UNLOCK)==0)
 { 
 APP_LOG_DEBUG("电子秤设备解锁...");
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_LOCK_MSG;
 scale_msg.scale=0;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_UNLOCK_VALUE;
 /*向电子秤功能任务发送解锁消息*/
 while(time<DEBUG_TASK_UNLOCK_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_LOCK_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_LOCK_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time > DEBUG_TASK_UNLOCK_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤解锁超时.");
 }
 else
 {
 APP_LOG_INFO("电子秤设备解锁成功.");
 }
 continue;
 }
 /*设备加锁*/
  if(strcmp((const char*)cmd,DEBUG_TASK_CMD_DEVICE_UNLOCK)==0)
 { 
 APP_LOG_DEBUG("电子秤设备解锁...");
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_LOCK_MSG;
 scale_msg.scale=0;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_UNLOCK_VALUE;
 /*向电子秤功能任务发送解锁消息*/
 while(time<DEBUG_TASK_UNLOCK_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_LOCK_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status!=osEventSignal || !(signal.value.signals & DEBUG_TASK_LOCK_OK_SIGNAL))/*超时或者其他错误*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time > DEBUG_TASK_UNLOCK_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤解锁超时.");
 }
 else
 {
 APP_LOG_INFO("电子秤设备解锁成功.");
 }
 continue;
 }

 /*设置最大称重值*/
 APP_LOG_INFO("电子秤设置最大称重值...");
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_SET_MAX_WEIGHT_MSG;
 scale_msg.scale=0;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_MAX_WEIGHT_VALUE;
 /*向电子秤功能任务发送设置最大称重值消息*/
 while(time<SCALE_POLL_TASK_SET_MAX_WEIGHT_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(SCALE_POLL_TASK_SET_MAX_WEIGHT_OK_SIGNAL,SCALE_POLL_TASK_WAIT_TIMEOUT);
 if(signal.status!=osEventSignal || !(signal.value.signals & SCALE_POLL_TASK_SET_MAX_WEIGHT_OK_SIGNAL))/*超时或者其他错误*/
 break;
 time+=SCALE_POLL_TASK_WAIT_TIMEOUT;
 }
 if(time > SCALE_POLL_TASK_SET_MAX_WEIGHT_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤设置最大称重值超时.再次尝试.");
  goto scale_init_handle;
 }
 APP_LOG_INFO("电子秤设置最大称重值成功.");
 
 APP_LOG_INFO("电子秤设置分辨率...");
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_SET_DIVISION_MSG;
 scale_msg.scale=0;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_DIVISION_VALUE;
 /*向电子秤功能任务发送设置最大称重值消息*/
 while(time<SCALE_POLL_TASK_SET_DIVISION_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(SCALE_POLL_TASK_SET_DIVISION_OK_SIGNAL,SCALE_POLL_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & SCALE_POLL_TASK_SET_DIVISION_OK_SIGNAL)/*超时或者其他错误*/
 break;
 time+=SCALE_POLL_TASK_WAIT_TIMEOUT;
 }
 if(time > SCALE_POLL_TASK_SET_DIVISION_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤设置最大分辨率超时.再次尝试.");
  goto scale_init_handle;
 }
 APP_LOG_INFO("电子秤设置最大分辨率成功.");
 
  
  }
 }
  
  
  
  
  
  
  
  
  
  
  
  
  