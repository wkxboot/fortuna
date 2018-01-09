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
 osEvent signal;
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
  uint8_t origin_addr,now_addr,len;
  uint16_t time;
  if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_ADDR,sizeof(DEBUG_TASK_CMD_SET_ADDR)-DEBUG_TASK_CMD_EOL_LEN)==0)
  {
  len=sizeof(DEBUG_TASK_CMD_SET_ADDR);
  origin_addr=cmd[len];
  now_addr=cmd[len+1];
  if(origin_addr > '9' || origin_addr <'1' ||now_addr > '9' || now_addr <'1' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_SET_ADDR)+DEBUG_TASK_CMD_SET_ADDR_PARAM_LEN)
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
  if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_SET_ADDR_OK_SIGNAL)/*超时或者其他错误*/
  break;
  time+=DEBUG_TASK_WAIT_TIMEOUT;
  }
 if(time > DEBUG_TASK_CMD_SET_ADDR_TIMEOUT)
 {
 APP_LOG_ERROR("设置地址超时错误.\r\n");  
 }
 }

  /*获取净重值*/
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT,sizeof(DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT)-DEBUG_TASK_CMD_EOL_LEN)==0)
 {
 len=sizeof(DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT);
 origin_addr=cmd[len];
 if(origin_addr > '9' || origin_addr <'0' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT)+DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_PARAM_LEN)
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤%2d获取净重...",origin_addr);
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG;
 scale_msg.scale=origin_addr;
 /*向电子秤功能任务发送获取净重消息*/
 while(time<DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL)/*超时或者其他错误*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time >= DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤获取净重超时错误.\r\n");
 }
 else
 {
 APP_LOG_INFO("电子秤设备获取净重成功.\r\n");
 }
 continue; 
 }
 /*设备解锁*/
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_UNLOCK_DEVICE,sizeof(DEBUG_TASK_CMD_UNLOCK_DEVICE)-DEBUG_TASK_CMD_EOL_LEN)==0)
 { 
 len=sizeof(DEBUG_TASK_CMD_UNLOCK_DEVICE);
 origin_addr=cmd[len];
 if(origin_addr > '9' || origin_addr <'0' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_UNLOCK_DEVICE)+DEBUG_TASK_CMD_UNLOCK_DEVICE_PARAM_LEN)
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设备解锁...");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_LOCK_MSG;
 scale_msg.scale=origin_addr;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_UNLOCK_VALUE;
 /*向电子秤功能任务发送解锁消息*/
 while(time<DEBUG_TASK_CMD_UNLOCK_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_UNLOCK_DEVICE_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_UNLOCK_DEVICE_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time > DEBUG_TASK_CMD_UNLOCK_TIMEOUT)
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
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_LOCK_DEVICE,sizeof(DEBUG_TASK_CMD_LOCK_DEVICE)-DEBUG_TASK_CMD_EOL_LEN)==0)
 { 
 len=sizeof(DEBUG_TASK_CMD_UNLOCK_DEVICE);
 origin_addr=cmd[len];
 if(origin_addr > '9' || origin_addr <'0' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_LOCK_DEVICE)+DEBUG_TASK_CMD_LOCK_DEVICE_PARAM_LEN)
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设备上锁...");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_LOCK_MSG;
 scale_msg.scale=origin_addr;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_LOCK_VALUE;
 /*向电子秤功能任务发送解锁消息*/
 while(time<DEBUG_TASK_CMD_LOCK_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_LOCK_DEVICE_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_LOCK_DEVICE_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time > DEBUG_TASK_CMD_LOCK_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤上锁超时.");
 }
 else
 {
  APP_LOG_INFO("电子秤设备上锁成功.");
 }
 continue;
 }

 /*设置最大称重值*/
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_MAX_WEIGHT,sizeof(DEBUG_TASK_CMD_SET_MAX_WEIGHT)-DEBUG_TASK_CMD_EOL_LEN)==0)
 { 
 len=sizeof(DEBUG_TASK_CMD_SET_MAX_WEIGHT)-DEBUG_TASK_CMD_EOL_LEN;
 origin_addr=cmd[len];
 if(origin_addr > '9' || origin_addr <'0' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_LOCK_DEVICE)+DEBUG_TASK_CMD_SET_MAX_WEIGHT_PARAM_LEN)
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置最大称重值...");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_SET_MAX_WEIGHT_MSG;
 scale_msg.scale=origin_addr;/*电子秤地址*/
 scale_msg.param16=SCALE_FUNC_TASK_MAX_WEIGHT_VALUE;
 /*向电子秤功能任务发送设置最大称重值消息*/
 while(time<DEBUG_TASK_CMD_SET_MAX_WEIGHT_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_SET_MAX_WEIGHT_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_SET_MAX_WEIGHT_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time >= DEBUG_TASK_CMD_SET_MAX_WEIGHT_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤设置最大称重值超时.\r\n");
 }
 else
 {
  APP_LOG_INFO("电子秤设置最大称重值成功.\r\n");
 }
 continue;
 }
 /*设置分度值*/
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_DIVISION,sizeof(DEBUG_TASK_CMD_SET_DIVISION)-2)==0)
 { 
 len=sizeof(DEBUG_TASK_CMD_SET_MAX_WEIGHT)-2;
 origin_addr=cmd[len];
 if(origin_addr > '9' || origin_addr <'0' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_SET_DIVISION)+DEBUG_TASK_CMD_SET_DIVISION_PARAM_LEN)
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置分度值...");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_SET_DIVISION_MSG;
 scale_msg.scale=origin_addr;/*电子秤地址*/
 scale_msg.param16=SCALE_FUNC_TASK_DIVISION_VALUE;
 /*向电子秤功能任务发送设置分度值消息*/
 while(time<DEBUG_TASK_CMD_SET_DIVISION_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_SET_DIVISION_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_SET_DIVISION_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time >= DEBUG_TASK_CMD_SET_DIVISION_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤设置分度值超时.\r\n");
 }
 else
 {
  APP_LOG_INFO("电子秤设置分度值成功.\r\n");
 }
 continue;
 } 
 /*设置最大称重值*/
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_MAX_WEIGHT,sizeof(DEBUG_TASK_CMD_SET_MAX_WEIGHT)-DEBUG_TASK_CMD_EOL_LEN)==0)
 { 
 len=sizeof(DEBUG_TASK_CMD_SET_MAX_WEIGHT)-DEBUG_TASK_CMD_EOL_LEN;
 origin_addr=cmd[len];
 if(origin_addr > '9' || origin_addr <'0' ||strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_LOCK_DEVICE)+1)
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置最大称重值...");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_SET_MAX_WEIGHT_MSG;
 scale_msg.scale=origin_addr;/*电子秤地址*/
 scale_msg.param16=SCALE_FUNC_TASK_MAX_WEIGHT_VALUE;
 /*向电子秤功能任务发送设置最大称重值消息*/
 while(time<DEBUG_TASK_CMD_SET_MAX_WEIGHT_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_SET_MAX_WEIGHT_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_SET_MAX_WEIGHT_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time >= DEBUG_TASK_CMD_SET_MAX_WEIGHT_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤设置最大称重值超时.\r\n");
 }
 else
 {
  APP_LOG_INFO("电子秤设置最大称重值成功.\r\n");
 }
 continue;
 }
 /*设置校准值*/
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_DIVISION,sizeof(DEBUG_TASK_CMD_SET_DIVISION)-DEBUG_TASK_CMD_EOL_LEN)==0)
 { 
 uint8_t i;
 uint16_t value;
 if(strlen((char const *)cmd)!=sizeof(DEBUG_TASK_CMD_SET_DIVISION)+DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN)
 APP_LOG_ERROR("命令长度非法.\r\n");
 value=0;
 for(i=0;i<DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN;i++)
 {
 if(cmd[len+i] <= '9' && cmd[len+i] >='0')
 {
 cmd[len+i]-='0';
 }
 else
 {
  APP_LOG_ERROR("重量值或者设备地址值非法.进制范围1-9.\r\n");
  break;
 }
 
 }
 /*发生了错误*/
 if(i!=DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN)
 continue;
 
 len=sizeof(DEBUG_TASK_CMD_SET_MAX_WEIGHT)-DEBUG_TASK_CMD_EOL_LEN; 
 origin_addr=cmd[len];

 value=cmd[len+1]*10000+cmd[len+2]*1000+cmd[len+3]*100+cmd[len+4]*10+cmd[len+5];

 APP_LOG_DEBUG("电子秤设置校准值...");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_CALIBRATE_WEIGHT_MSG;
 scale_msg.scale=origin_addr;/*电子秤地址*/
 scale_msg.param16=value;
 /*向电子秤功能任务发送校准重量消息*/
 while(time<DEBUG_TASK_CMD_CALIBRATE_WEIGHT_TIMEOUT)
 {
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_CALIBRATE_WEIGHT_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_CALIBRATE_WEIGHT_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time >= DEBUG_TASK_CMD_CALIBRATE_WEIGHT_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤校准重量超时.\r\n");
 }
 else
 {
  APP_LOG_INFO("电子秤校准重量成功.\r\n");
 }
 continue;
 }
 }
 }
  
  
  
  
  
  
  
  
  
  
  
  
  