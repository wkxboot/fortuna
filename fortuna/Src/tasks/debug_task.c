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
#include "mb_m.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[debug]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

#define  DEBUG_CMD_MAX_LEN      30

osThreadId debug_task_hdl;
uint8_t cmd[DEBUG_CMD_MAX_LEN];


static uint16_t data_cnt;
static uint8_t origin_addr,new_addr,offset,cmd_len,recv_len;
static uint8_t scale,scale_end,scale_start;

static void debug_task_check_addr(uint8_t origin_addr)
{
  if(origin_addr==0)/*对所有的称*/
  {
  scale_start=1;
  scale_end=SCALES_CNT_MAX;
  }
  else
  {
  scale_start=origin_addr; 
  scale_end=scale_start;  
  }
}

/*RTT调试任务*/
void debug_task(void const * argument)
{
 uint8_t debug_enable=FORTUNA_TRUE;

 uint16_t param[2];
 uint16_t reg_addr,reg_cnt;
 eMBMasterReqErrCode err_code;

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

  APP_LOG_DEBUG("%2d#电子秤设置地址...\r\n",origin_addr);
  param[0]=new_addr;/*填入地址参数*/    
  err_code=eMBMasterReqWriteMultipleHoldingRegister(origin_addr,DEVICE_ADDR_REG_ADDR,DEVICE_ADDR_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
    /*执行成功*/
  if(err_code==MB_MRE_NO_ERR)
  {
  APP_LOG_DEBUG("%2d#电子秤设置地址成功.\r\n",origin_addr);
  }
  APP_LOG_ERROR("%2d#电子秤设置地址失败.\r\n",scale); 
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
  APP_LOG_DEBUG("获取净重...\r\n");
  
  debug_task_check_addr(origin_addr);
  for(scale=scale_start;scale<=scale_end;scale++)
  {
  APP_LOG_DEBUG("%2d#电子秤获取净重...\r\n",scale);
  err_code=eMBMasterReqReadHoldingRegister(scale,DEVICE_NET_WEIGHT_REG_ADDR,DEVICE_NET_WEIGHT_REG_CNT,DEBUG_TASK_WAIT_TIMEOUT);
  /*执行成功*/
  if(err_code==MB_MRE_NO_ERR)
  {
  uint16_t net_weight;
  get_net_weight(scale,&net_weight);
  APP_LOG_DEBUG("%2d#电子秤获取净重成功.净重值：%dg.\r\n",net_weight);
  }
  else
  {
  APP_LOG_ERROR("%2d#电子秤获取净重失败.\r\n",scale);
  }
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

 param[0]=SCALE_UNLOCK_VALUE;
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤解锁...\r\n",scale);
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_LOCK_REG_ADDR,DEVICE_LOCK_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
    /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤解锁成功.\r\n",scale);
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤解锁失败.\r\n",scale);
 }
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

 param[0]=SCALE_LOCK_VALUE;
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤上锁...\r\n",scale);
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_LOCK_REG_ADDR,DEVICE_LOCK_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
    /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤上锁成功.\r\n",scale);
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤上锁失败.\r\n",scale);
 }
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
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围1-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤去皮...\r\n");
 param[0]=SCALE_AUTO_TARE_WEIGHT_VALUE>>16;
 param[1]=SCALE_AUTO_TARE_WEIGHT_VALUE&0xffff;
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤去皮...\r\n",scale);
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_TARE_WEIGHT_REG_ADDR,DEVICE_TARE_WEIGHT_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
 /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤去皮成功.\r\n",scale);
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤去皮失败.\r\n",scale);
 }
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
 
 param[0]=SCALE_ZERO_RANGE_VALUE;
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤设置清零范围...\r\n",scale);
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MANUALLY_CLEAR_RANGE_REG_ADDR,DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
 /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤设置清零范围成功.\r\n",scale);
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤设置清零范围失败.\r\n",scale);
 }
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
  APP_LOG_ERROR("命令长度或者设备地址值非法.地址范围0-9.\r\n");
  continue;
 }
 origin_addr-='0';
 APP_LOG_DEBUG("电子秤清零...\r\n");
 
 param[0]=SCALE_CLEAR_ZERO_VALUE;
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤清零...\r\n",scale);
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MANUALLY_CLEAR_REG_ADDR,DEVICE_MANUALLY_CLEAR_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
 /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤清零成功.\r\n",scale);
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤清零失败.\r\n",scale);
 }
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
 
 param[0]=0;
 param[1]=SCALE_MAX_WEIGHT_VALUE;
 
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤设置最大称重值...\r\n",scale);
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MAX_WEIGHT_REG_ADDR,DEVICE_MAX_WEIGHT_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
    /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤设置最大称重值成功.\r\n",scale);
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤设置最大称重值失败.\r\n",scale);
 }
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
 
 param[0]=SCALE_DIVISION_VALUE;
 
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤设置分度值...\r\n",scale);
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_DIVISION_REG_ADDR,DEVICE_DIVISION_REG_CNT,param,DEBUG_TASK_WAIT_TIMEOUT);
    /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤设置分度值成功.\r\n",scale);
 }
 else
 {
 APP_LOG_ERROR("%2d#电子秤设置分度值失败.\r\n",scale);
 }
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
 APP_LOG_DEBUG("电子秤校准...\r\n");
 
 param[0]=0;
 param[1]=value;
 if(param[1]==0)
 {
 reg_addr=DEVICE_ZERO_CALIBRATE_REG_ADDR;
 reg_cnt=DEVICE_ZERO_CALIBRATE_REG_CNT;
 APP_LOG_DEBUG("0点重量校准.\r\n");
 }
 else
 {
 reg_addr=DEVICE_SPAN_CALIBRATE_REG_ADDR;
 reg_cnt=DEVICE_SPAN_CALIBRATE_REG_CNT;
 APP_LOG_DEBUG("非0点重量校准.\r\n");
 }
 debug_task_check_addr(origin_addr);
 for(scale=scale_start;scale<=scale_end;scale++)
 {
 APP_LOG_DEBUG("%2d#电子秤校准...\r\n",scale); 
 err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,reg_addr,reg_cnt,param,DEBUG_TASK_WAIT_TIMEOUT);
    /*执行成功*/
 if(err_code==MB_MRE_NO_ERR)
 {
  APP_LOG_DEBUG("%2d#电子秤校准成功.\r\n",scale);
 }
 else
 {
  APP_LOG_ERROR("%2d#电子秤校准失败.\r\n",scale);
 }
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
  
  
  
  
  
  
  
  
  
  
  
  
  