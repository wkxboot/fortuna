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

#define  DEBUG_CMD_MAX_LEN      30

osThreadId debug_task_hdl;
uint8_t cmd[DEBUG_CMD_MAX_LEN];


uint16_t data_cnt;
uint8_t origin_addr,new_addr,offset,cmd_len,recv_len;
uint16_t time;
/*RTT调试任务*/
void debug_task(void const * argument)
{
 uint8_t debug_enable=FORTUNA_TRUE;
 osEvent signal;
 scale_msg_t scale_msg;

 APP_LOG_INFO("######调试任务开始.\r\n");

 while(1)
 {
  osDelay(DEBUG_TASK_WAIT_TIMEOUT); 
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
  recv_len=strlen((char const*)cmd)-DEBUG_TASK_CMD_EOL_LEN;
  
  cmd_len=strlen(DEBUG_TASK_CMD_SET_ADDR);
  /*设置地址*/
  if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_ADDR,cmd_len)==0)
  {
  offset=cmd_len;
  origin_addr=cmd[offset];
  new_addr=cmd[offset+1];
  if(recv_len!=cmd_len+DEBUG_TASK_CMD_SET_ADDR_PARAM_LEN || origin_addr > '9' || origin_addr <'1' ||new_addr > '9' || new_addr <'1' )
  {
   APP_LOG_ERROR("命令长度或者地址值非法.地址范围1-9.\r\n");
   continue;
  }
  origin_addr-='0';
  new_addr-='0';
  APP_LOG_DEBUG("原地址：%2d设置为%2d.\r\n",origin_addr,new_addr); 
  scale_msg.type=SCALE_FUNC_TASK_SET_ADDR_MSG;
  scale_msg.scale=origin_addr;
  scale_msg.param16=new_addr;
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
 if(time >=DEBUG_TASK_CMD_SET_ADDR_TIMEOUT)
 {
 APP_LOG_ERROR("设置地址超时错误.\r\n");  
 }
 continue;
 }

  /*获取净重值*/
 cmd_len=strlen(DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT,cmd_len)==0)
 {
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len!=cmd_len+DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤%2d获取净重...\r\n",origin_addr);
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG;
 scale_msg.scale=origin_addr;
 /*向电子秤功能任务发送获取净重消息*/
 while(time<DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
 APP_LOG_DEBUG("向电子秤功能任务发送获取净重消息.\r\n");
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
 cmd_len=strlen(DEBUG_TASK_CMD_UNLOCK_DEVICE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_UNLOCK_DEVICE,cmd_len)==0)
 { 
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len != cmd_len +DEBUG_TASK_CMD_UNLOCK_DEVICE_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设备解锁...\r\n");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_LOCK_MSG;
 scale_msg.scale=origin_addr;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_UNLOCK_VALUE;
 /*向电子秤功能任务发送解锁消息*/
 while(time<DEBUG_TASK_CMD_UNLOCK_TIMEOUT)
 {
 APP_LOG_DEBUG("向电子秤功能任务发送解锁消息.\r\n");
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_UNLOCK_DEVICE_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_UNLOCK_DEVICE_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time >= DEBUG_TASK_CMD_UNLOCK_TIMEOUT)
 {
  APP_LOG_ERROR("电子秤解锁超时.\r\n");
 }
 else
 {
 APP_LOG_INFO("电子秤设备解锁成功.\r\n");
 }
 continue;
 }
 /*设备加锁*/
 cmd_len=strlen(DEBUG_TASK_CMD_LOCK_DEVICE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_LOCK_DEVICE,cmd_len)==0)
 { 
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len != cmd_len+DEBUG_TASK_CMD_LOCK_DEVICE_PARAM_LEN ||origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设备上锁...\r\n");
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_LOCK_MSG;
 scale_msg.scale=origin_addr;/*全部的电子秤*/
 scale_msg.param16=SCALE_FUNC_TASK_LOCK_VALUE;
 /*向电子秤功能任务发送解锁消息*/
 while(time<DEBUG_TASK_CMD_LOCK_TIMEOUT)
 {
 APP_LOG_DEBUG("向电子秤功能任务发送上锁消息.\r\n");
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(DEBUG_TASK_LOCK_DEVICE_OK_SIGNAL,DEBUG_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & DEBUG_TASK_LOCK_DEVICE_OK_SIGNAL)/*收到ok信号*/
 break;
 time+=DEBUG_TASK_WAIT_TIMEOUT;
 }
 if(time >= DEBUG_TASK_CMD_LOCK_TIMEOUT)
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
 cmd_len=strlen(DEBUG_TASK_CMD_SET_MAX_WEIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_MAX_WEIGHT,cmd_len)==0)
 { 
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len !=cmd_len + DEBUG_TASK_CMD_SET_MAX_WEIGHT_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置最大称重值...\r\n");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_SET_MAX_WEIGHT_MSG;
 scale_msg.scale=origin_addr;/*电子秤地址*/
 scale_msg.param16=SCALE_FUNC_TASK_MAX_WEIGHT_VALUE;
 /*向电子秤功能任务发送设置最大称重值消息*/
 while(time<DEBUG_TASK_CMD_SET_MAX_WEIGHT_TIMEOUT)
 {
 APP_LOG_DEBUG("向电子秤功能任务发送设置最大值消息.\r\n");
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
 cmd_len=strlen(DEBUG_TASK_CMD_SET_DIVISION);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_SET_DIVISION,cmd_len)==0)
 { 
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len != cmd_len +DEBUG_TASK_CMD_SET_DIVISION_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置分度值...\r\n");
 
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_SET_DIVISION_MSG;
 scale_msg.scale=origin_addr;/*电子秤地址*/
 scale_msg.param16=SCALE_FUNC_TASK_DIVISION_VALUE;
 /*向电子秤功能任务发送设置分度值消息*/
 while(time<DEBUG_TASK_CMD_SET_DIVISION_TIMEOUT)
 {
 APP_LOG_DEBUG("向电子秤功能任务发送设置分度消息.\r\n");
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
 /*设置校准值*/
 cmd_len=strlen(DEBUG_TASK_CALIBRATE_WEIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CALIBRATE_WEIGHT,cmd_len)==0)
 { 
 uint16_t value;
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN)
 APP_LOG_ERROR("命令长度非法.\r\n");
 value=0;
 offset=cmd_len;
 for(uint8_t i=0;i<DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN;i++)
 {
 if(cmd[offset+i] <= '9' && cmd[offset+i] >='0')
 {
 cmd[offset+i]-='0';
 }
 else /*发生了错误*/
 {
 APP_LOG_ERROR("重量值或者设备地址值非法.进制范围1-9.\r\n");
 goto err_handle;
 }
 }
 origin_addr=cmd[offset];
 value=cmd[offset+1]*10000+cmd[offset+2]*1000+cmd[offset+3]*100+cmd[offset+4]*10+cmd[offset+5];
 APP_LOG_DEBUG("电子秤设置校准值...\r\n");
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_CALIBRATE_WEIGHT_MSG;
 scale_msg.scale=origin_addr;/*电子秤地址*/
 scale_msg.param16=value;
 /*向电子秤功能任务发送校准重量消息*/
 while(time<DEBUG_TASK_CMD_CALIBRATE_WEIGHT_TIMEOUT)
 {
 APP_LOG_DEBUG("向电子秤功能任务发送设置校准值消息.\r\n");
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
err_handle:
 continue;
 }
 }
 }
  
  
  
  
  
  
  
  
  
  
  
  
  