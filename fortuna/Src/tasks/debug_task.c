#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "string.h"
#include "fortuna_common.h"
#include "scales.h"
#include "comm_protocol.h"
#include "host_comm_task.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#include "scale_func_task.h"
#include "lock_task.h"
#include "compressor_task.h"
#include "temperature_task.h"
#include "dc12v_task.h"
#include "light_task.h"
#include "glass_pwr_task.h"
#include "debug_task.h"
#include "ABDK_ZNHG_ZK.h"
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
 cmd_len=strlen(DEBUG_TASK_CMD_CALIBRATE_WEIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_CALIBRATE_WEIGHT,cmd_len)==0)
 { 
 uint16_t value;
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN)
 APP_LOG_ERROR("校准命令长度非法.\r\n");
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
 /*打开压缩机*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_COMPRESSOR);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_COMPRESSOR,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_COMPRESSOR_PARAM_LEN)
 {
   APP_LOG_ERROR("压缩机命令长度非法.\r\n");
 continue;
 } 
 /*向压缩机任务发送开机信号*/
 APP_LOG_DEBUG("向压缩机任务发送开机信号.\r\n");
 osSignalSet(compressor_task_hdl,COMPRESSOR_TASK_PWR_ON_SIGNAL);
 continue;
 }
 /*关闭压缩机*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR_PARAM_LEN)
 {
 APP_LOG_ERROR("压缩机命令长度非法.\r\n");
 continue;
 } 
 /*向压缩机任务发送关机信号*/
 APP_LOG_DEBUG("向压缩机任务发送关机信号.\r\n");
 osSignalSet(compressor_task_hdl,COMPRESSOR_TASK_PWR_OFF_SIGNAL);
 continue;
 }
 /*打开所有灯带*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_LIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_LIGHT,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_LIGHT_PARAM_LEN)
 {
 APP_LOG_ERROR("灯带命令长度非法.\r\n");
 continue;
 } 
 /*向灯带任务发送打开信号*/
 APP_LOG_DEBUG("向灯带任务发送打开信号.\r\n");
 osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_1_PWR_ON_SIGNAL|LIGHT_TASK_LIGHT_2_PWR_ON_SIGNAL);
 continue;
 }
 /*关闭所有灯带*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_LIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_LIGHT,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_LIGHT_PARAM_LEN)
 {
 APP_LOG_ERROR("灯带命令长度非法.\r\n");
 continue;
 } 
 /*向灯带任务发送关闭信号*/
 APP_LOG_DEBUG("向灯带任务发送关闭信号.\r\n");
 osSignalSet(light_task_hdl,LIGHT_TASK_LIGHT_1_PWR_OFF_SIGNAL|LIGHT_TASK_LIGHT_2_PWR_OFF_SIGNAL);
 continue;
 } 
 
 /*打开所有12V*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_12V);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_12V,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_12V_PARAM_LEN)
 {
 APP_LOG_ERROR("12V命令长度非法.\r\n");
 continue;
 } 
 /*向12V任务发送打开信号*/
 APP_LOG_DEBUG("向12V任务发送打开信号.\r\n");
 osSignalSet(dc12v_task_hdl,DC12V_TASK_12V_1_PWR_ON_SIGNAL|DC12V_TASK_12V_2_PWR_ON_SIGNAL);
 continue;
 } 
 /*关闭所有12V*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_12V);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_12V,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_12V_PARAM_LEN)
 {
 APP_LOG_ERROR("12V命令长度非法.\r\n");
 continue;
 } 
 /*向12V任务发送关闭信号*/
 APP_LOG_DEBUG("向12V任务发送关闭信号.\r\n");
 osSignalSet(dc12v_task_hdl,DC12V_TASK_12V_1_PWR_OFF_SIGNAL|DC12V_TASK_12V_2_PWR_OFF_SIGNAL);
 continue;
 } 
 /*打开玻璃电源*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_GLASS);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_GLASS,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_GLASS_PARAM_LEN)
 {
 APP_LOG_ERROR("命令长度非法.\r\n");
 continue;
 } 
 /*玻璃电源任务发送打开信号*/
 APP_LOG_DEBUG("玻璃电源任务发送打开信号.\r\n");
 osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_ON_SIGNAL);
 continue;
 } 
  /*关闭玻璃电源*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_GLASS);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_GLASS,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_GLASS_PARAM_LEN)
 {
 APP_LOG_ERROR("命令长度非法.\r\n");
 continue;
 } 
 /*玻璃电源任务发送关闭信号*/
 APP_LOG_DEBUG("玻璃电源任务发送关闭信号.\r\n");
 osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_OFF_SIGNAL);
 continue;
 } 
 
 /*获取温度值*/
 cmd_len=strlen(DEBUG_TASK_CMD_OBTAIN_TEMPERATURE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_OBTAIN_TEMPERATURE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_OBTAIN_TEMPERATURE_PARAM_LEN)
 {
  APP_LOG_ERROR("温度命令长度非法.\r\n");
  continue;
 }
 int8_t t;
 offset=cmd_len;
 if(cmd[offset]!='0')/*0代表所有温度计平均值*/
   t=get_average_temperature();
 else if(cmd[offset]!='1')
   t=get_temperature(0);
 else if(cmd[offset]!='2')
   t=get_temperature(1);
 else
 {
  APP_LOG_ERROR("温度命令参数%2d非法.0-1-2之一.\r\n",cmd[offset]);
  continue;  
 }
 APP_LOG_DEBUG("温度值：%2d.\r\n",t);
 continue;
 } 
 /*打开交流电*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_AC);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_AC,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_AC_PARAM_LEN)
 {
  APP_LOG_ERROR("交流电命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("打开交流电.\r\n");
 BSP_AC_TURN_ON_OFF(AC_1|AC_2,AC_CTL_ON);
 continue;
 } 
 /*关闭交流电*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_AC);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_AC,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_AC_PARAM_LEN)
 {
  APP_LOG_ERROR("交流电命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("关闭交流电.\r\n");
 BSP_AC_TURN_ON_OFF(AC_1|AC_2,AC_CTL_OFF);
 continue;
 }
  /*开锁*/
 cmd_len=strlen(DEBUG_TASK_CMD_UNLOCK_LOCK);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_UNLOCK_LOCK,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_UNLOCK_LOCK_PARAM_LEN)
 {
  APP_LOG_ERROR("开锁命令长度非法.\r\n");
  continue;
 }
 lock_msg_t msg;
 msg.type=LOCK_TASK_UNLOCK_LOCK_MSG;
 APP_LOG_DEBUG("向锁任务发送开锁消息.\r\n");
 osMessagePut(lock_task_msg_q_id,*(uint32_t*)&msg,0);
 continue;
 }
 /*关锁*/
 cmd_len=strlen(DEBUG_TASK_CMD_LOCK_LOCK);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_LOCK_LOCK,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_LOCK_LOCK_PARAM_LEN)
 {
  APP_LOG_ERROR("关锁命令长度非法.\r\n");
  continue;
 }
 lock_msg_t msg;
 msg.type=LOCK_TASK_LOCK_LOCK_MSG;
 APP_LOG_DEBUG("向锁任务发送关锁消息.\r\n");
 osMessagePut(lock_task_msg_q_id,*(uint32_t*)&msg,0);
 continue;
 }
 /*打开门指示灯*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_LOCK_LED);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_LOCK_LED,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_LOCK_LED_PARAM_LEN)
 {
  APP_LOG_ERROR("开门指示灯命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("打开门所有指示灯.\r\n");
 BSP_LED_TURN_ON_OFF(DOOR_RED_LED|DOOR_GREEN_LED|DOOR_ORANGE_LED,LED_CTL_ON);
 continue;
 }
 /*关闭门指示灯*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_LOCK_LED);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_LOCK_LED,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_LOCK_LED_PARAM_LEN)
 {
  APP_LOG_ERROR("关闭门指示灯命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("关闭门所有指示灯.\r\n");
 BSP_LED_TURN_ON_OFF(DOOR_RED_LED|DOOR_GREEN_LED|DOOR_ORANGE_LED,LED_CTL_OFF);
 continue;
 }
 
 
 }
 }
  
  
  
  
  
  
  
  
  
  
  
  
  