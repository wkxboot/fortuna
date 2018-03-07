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
#include "ups_task.h"
#include "debug_task.h"
#include "cpu_utils.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[debug]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

#define  DEBUG_CMD_MAX_LEN      30

osThreadId debug_task_hdl;
uint8_t cmd[DEBUG_CMD_MAX_LEN];


static uint16_t data_cnt;
static uint8_t origin_addr,offset,cmd_len,recv_len;


/*RTT调试任务*/
void debug_task(void const * argument)
{
 fortuna_bool_t debug_enable=FORTUNA_TRUE;

 fortuna_bool_t ret;

 APP_LOG_INFO("######调试任务开始.\r\n");

 while(1)
 {
  osDelay(DEBUG_TASK_WAIT_TIMEOUT); 
  data_cnt=SEGGER_RTT_HasData(0);
  /*buff0没有数据或者有超出最大数量值的数据*/
  if(data_cnt==0)
    continue;  
  data_cnt=SEGGER_RTT_Read(0,cmd,DEBUG_CMD_MAX_LEN);
  if(debug_enable!=FORTUNA_TRUE)
    continue;
  APP_LOG_DEBUG("读的字节数：%d\r\n",data_cnt);  
  if(data_cnt>DEBUG_CMD_MAX_LEN-1)
    data_cnt=DEBUG_CMD_MAX_LEN-1;
  cmd[data_cnt]=0;/*填充为完整字符串*/
  recv_len=strlen((char const*)cmd)-DEBUG_TASK_CMD_EOL_LEN;
  
  /*获取净重值*/
 cmd_len=strlen(DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT,cmd_len)==0)
 {
  offset=cmd_len;
  origin_addr=cmd[offset];
  if(recv_len!=cmd_len+DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
  {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
  }
  origin_addr-='0'; 
  APP_LOG_DEBUG("获取净重...\r\n"); 
  ret=scale_obtain_net_weight(origin_addr,0);
  /*执行成功*/
  if(ret==FORTUNA_TRUE)
  {
  int16_t net_weight[SCALES_CNT_MAX];
  get_net_weight(0,net_weight);
  APP_LOG_DEBUG("电子秤获取净重成功.\r\n");
  for(uint8_t i=0;i< SCALES_CNT_MAX;i++)
  {
  APP_LOG_ARRAY("%d#称净重值：%dg.\r\n",i+1,net_weight[i]);
  }
  }
  else
  {
  APP_LOG_ERROR("电子秤获取净重失败.\r\n");
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
  APP_LOG_ERROR("解锁命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设备解锁...\r\n");

 ret=scale_lock_operation(origin_addr,SCALE_UNLOCK_VALUE);
  /*执行成功*/
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤解锁成功.\r\n");
 }
 else
 {
 APP_LOG_ERROR("电子秤解锁失败.\r\n");
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
  APP_LOG_ERROR("上命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设备上锁...\r\n");

 ret=scale_lock_operation(origin_addr,SCALE_LOCK_VALUE);
  /*执行成功*/
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤上锁成功.\r\n");
 }
 else
 {
 APP_LOG_ERROR("电子秤上锁失败.\r\n");
 }
 continue;
 }
 /*去皮*/
 cmd_len=strlen(DEBUG_TASK_CMD_REMOVE_TARE_WEIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_REMOVE_TARE_WEIGHT,cmd_len)==0)
 { 
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len != cmd_len +DEBUG_TASK_CMD_REMOVE_TARE_WEIGHT_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("去皮命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤去皮...\r\n");
 ret=scale_remove_tare(origin_addr,SCALE_AUTO_TARE_WEIGHT_VALUE);
  /*执行成功*/
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤去皮成功.\r\n");
 }
 else
 {
 APP_LOG_ERROR("电子秤去皮失败.\r\n");
 }
 continue;
 }
 /*设置手动清零范围*/
 cmd_len=strlen(DEBUG_TASK_CMD_ZERO_RANGE_SET);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_ZERO_RANGE_SET,cmd_len)==0)
 { 
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len != cmd_len +DEBUG_TASK_CMD_ZERO_RANGE_SET_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置清零范围...\r\n");
 
 ret=scale_manully_zero_range(origin_addr,SCALE_ZERO_RANGE_VALUE);
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("%2d#电子秤设置清零范围成功.\r\n");
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤设置清零范围失败.\r\n");
 }
 continue;
 }
 /*手动清零*/
 cmd_len=strlen(DEBUG_TASK_CMD_CLEAR_ZERO);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_CLEAR_ZERO,cmd_len)==0)
 { 
 offset=cmd_len;
 origin_addr=cmd[offset];
 if(recv_len != cmd_len +DEBUG_TASK_CMD_CLEAR_ZERO_PARAM_LEN || origin_addr > '9' || origin_addr <'0')
 {
  APP_LOG_ERROR("清零命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤清零...\r\n");
 ret=scale_remove_tare(origin_addr,0);/*首先设置皮重为0*/
 if(ret==FORTUNA_TRUE)
 {
 ret=scale_clear_zero(origin_addr,0);
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤清零成功.\r\n");
 }
 else
 {
 APP_LOG_ERROR("电子秤清零失败.\r\n");
 }
 }
 else
 {
 APP_LOG_ERROR("电子秤清零失败.\r\n");
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
  APP_LOG_ERROR("设置最大值命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置最大称重值...\r\n");
 
 ret=scale_set_max_weight(origin_addr,SCALE_MAX_WEIGHT_VALUE);
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤设置最大称重值成功.\r\n");
 }
 else
 {
 APP_LOG_ERROR("电子秤设置最大称重值失败.\r\n");
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
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤设置分度值...\r\n");
 
 ret=scale_set_division(origin_addr,SCALE_DIVISION_VALUE);
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤设置分度值成功.\r\n");
 }
 else
 {
 APP_LOG_ERROR("电子秤设置分度值失败.\r\n");
 }
 continue;
 }
 
/*标定内码值*/
 cmd_len=strlen(DEBUG_TASK_CMD_CALIBRATE_CODE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_CALIBRATE_CODE,cmd_len)==0)
 { 
 uint16_t value;
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_CALIBRATE_CODE_PARAM_LEN)
 APP_LOG_ERROR("内码命令长度非法.\r\n");
 value=0;
 offset=cmd_len;
 for(uint8_t i=0;i<DEBUG_TASK_CMD_CALIBRATE_CODE_PARAM_LEN;i++)
 {
 if(cmd[offset+i] <= '9' && cmd[offset+i] >='0')
 {
 cmd[offset+i]-='0';
 }
 else /*发生了错误*/
 {
 APP_LOG_ERROR("内码值或者设备地址值非法.进制范围1-9.\r\n");
 goto err_handle0;
 }
 }
 origin_addr=cmd[offset];
 value=cmd[offset+1];
 APP_LOG_DEBUG("电子秤标定内码值...\r\n");
 
 ret=scale_calibrate_code(origin_addr,value);
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤标定内码值成功.\r\n");
 }
 else
 {
  APP_LOG_ERROR("电子秤标定内码值失败.\r\n");
 }
err_handle0:
 continue;
 }
 /*标定测量值*/
 cmd_len=strlen(DEBUG_TASK_CMD_CALIBRATE_MEASUREMENT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_CALIBRATE_MEASUREMENT,cmd_len)==0)
 { 
 uint16_t value;
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_CALIBRATE_MEASUREMENT_PARAM_LEN)
 APP_LOG_ERROR("标定测量值命令长度非法.\r\n");
 value=0;
 offset=cmd_len;
 for(uint8_t i=0;i<DEBUG_TASK_CMD_CALIBRATE_MEASUREMENT_PARAM_LEN;i++)
 {
 if(cmd[offset+i] <= '9' && cmd[offset+i] >='0')
 {
 cmd[offset+i]-='0';
 }
 else /*发生了错误*/
 {
 APP_LOG_ERROR("测量值测量值或者设备地址值非法.进制范围0-9.\r\n");
 goto err_handle1;
 }
 }
 origin_addr=cmd[offset];
 value=cmd[offset+1]*10000+cmd[offset+2]*1000+cmd[offset+3]*100+cmd[offset+4]*10+cmd[offset+5];
 APP_LOG_DEBUG("电子秤标定测量值...\r\n");
 
 ret=scale_calibrate_measurement(origin_addr,value);
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤标定测量值成功.\r\n");
 }
 else
 {
  APP_LOG_ERROR("电子秤标定测量值失败.\r\n");
 }
err_handle1:
 continue;
 }
 /*标定重量值*/
 cmd_len=strlen(DEBUG_TASK_CMD_CALIBRATE_WEIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_CALIBRATE_WEIGHT,cmd_len)==0)
 { 
 uint16_t value;
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN)
 APP_LOG_ERROR("标定重量值命令长度非法.\r\n");
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
 APP_LOG_ERROR("标定重量值或者设备地址值非法.进制范围1-9.\r\n");
 goto err_handle2;
 }
 }
 origin_addr=cmd[offset];
 value=cmd[offset+1]*10000+cmd[offset+2]*1000+cmd[offset+3]*100+cmd[offset+4]*10+cmd[offset+5];
 APP_LOG_DEBUG("电子秤标定重量值...\r\n");
 
 ret=scale_calibrate_weight(origin_addr,value);
 if(ret==FORTUNA_TRUE)
 {
  APP_LOG_DEBUG("电子秤标定重量值成功.\r\n");
 }
 else
 {
  APP_LOG_ERROR("电子秤标定重量值成功.\r\n");
 }
err_handle2:
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
 
 /*获取UPS状态*/
 cmd_len=strlen(DEBUG_TASK_CMD_GET_UPS_STATE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_GET_UPS_STATE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_GET_UPS_STATE_PARAM_LEN)
 {
 APP_LOG_ERROR("UPS状态命令长度非法.\r\n");
 continue;
 } 
 /*获取UPS状态*/
 uint8_t state;
 state=get_ups_state();
 if(state==UPS_TASK_STATE_PWR_ON)
 {
  APP_LOG_DEBUG("UPS状态--接通市电.\r\n");
 }
 else
 {
  APP_LOG_DEBUG("UPS状态--断开市电.\r\n");
 }
 continue;
 }
 /*获取锁状态*/
 cmd_len=strlen(DEBUG_TASK_CMD_GET_LOCK_STATE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_GET_LOCK_STATE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_GET_LOCK_STATE_PARAM_LEN)
 {
 APP_LOG_ERROR("锁状态命令长度非法.\r\n");
 continue;
 } 
 /*获取锁状态*/
 uint8_t state;
 state=get_lock_state();
 if(state==LOCK_TASK_STATE_LOCKED)
 {
  APP_LOG_DEBUG("锁状态--关闭.\r\n");
 }
 else
 {
 APP_LOG_DEBUG("锁状态--打开.\r\n");
 }
 continue;
 } 
  /*获取门状态*/
 cmd_len=strlen(DEBUG_TASK_CMD_GET_DOOR_STATE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_GET_DOOR_STATE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_GET_DOOR_STATE_PARAM_LEN)
 {
 APP_LOG_ERROR("门状态命令长度非法.\r\n");
 continue;
 } 
 /*获取门状态*/
 uint8_t state;
 state=get_door_state();
 if(state==LOCK_TASK_STATE_LOCKED)
 {
  APP_LOG_DEBUG("门状态--关闭.\r\n");
 }
 else
 {
 APP_LOG_DEBUG("门状态--打开.\r\n");
 }
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
 if(cmd[offset]=='0')/*0代表所有温度计平均值*/
 t=get_average_temperature();
 else if(cmd[offset]=='1')
 t=get_temperature(0);
 else if(cmd[offset]=='2')
 t=get_temperature(1);
 else
 {
  APP_LOG_ERROR("温度命令参数%2d非法.0-1-2之一.\r\n",cmd[offset]);
  continue;  
 }
 APP_LOG_DEBUG("温度值：%2d 摄氏度.\r\n",t);
 continue;
 } 
 /*打开交流电1*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_AC1);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_AC1,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_AC1_PARAM_LEN)
 {
  APP_LOG_ERROR("交流电1命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("打开交流电1.\r\n");
 BSP_AC_TURN_ON_OFF(AC_1,AC_CTL_ON);
 continue;
 } 
 /*关闭交流电1*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_AC1);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_AC1,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_AC1_PARAM_LEN)
 {
  APP_LOG_ERROR("交流电1命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("关闭交流电1.\r\n");
 BSP_AC_TURN_ON_OFF(AC_1,AC_CTL_OFF);
 continue;
 }
 /*打开交流电2*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_AC2);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_AC2,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_AC2_PARAM_LEN)
 {
  APP_LOG_ERROR("交流电2命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("打开交流电2.\r\n");
 BSP_AC_TURN_ON_OFF(AC_2,AC_CTL_ON);
 continue;
 } 
 /*关闭交流电2*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_AC2);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_AC2,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_AC2_PARAM_LEN)
 {
  APP_LOG_ERROR("交流电2命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("关闭交流电2.\r\n");
 BSP_AC_TURN_ON_OFF(AC_2,AC_CTL_OFF);
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
 
  /*cpu负载查询*/
 cmd_len=strlen(DEBUG_TASK_CMD_CPU_LOAD);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_CPU_LOAD,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_CPU_LOAD_PARAM_LEN)
 {
  APP_LOG_ERROR("查询cpu负载命令长度非法.\r\n");
  continue;
 }
 
 APP_LOG_DEBUG("CPU负载：%d %%.\r\n",osGetCPUUsage());
 continue;
 }
 
 }
 }
  
  
  
  
  
  
  
  
  
  
  
  
  