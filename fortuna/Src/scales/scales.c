#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"   
#include "scales.h"
#include "modbus_poll.h"
#define APP_LOG_MODULE_NAME   "[scales]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

typedef struct 
{
 uint8_t id;
 uint8_t timeout_cnt;/*超时次数*/
 int16_t net_weight;/*净重*/
}scale_info_t;

/*电子传感器信息*/
scale_info_t scale[SCALES_CNT_MAX];
/*版本信息*/
uint16_t firmware_version[SCALES_CNT_MAX];


/*初始化电子秤传感器信息*/
void scale_init()
{
  for(uint8_t i=0;i<SCALES_CNT_MAX;i++)
  {
  scale[i].id=i+1;
  scale[i].net_weight=0;
  scale[i].timeout_cnt=0;
  }
}

/*搜索电子秤传感器*/
scale_info_t *seek_scale(uint8_t id)
{
 for(uint8_t i=0;i<SCALES_CNT_MAX;i++)
 {
  if(scale[i].id==id)
  return &scale[i];
 }
 return NULL;
}

/*获取净重值*/
fortuna_bool_t get_net_weight(uint8_t id,int16_t *ptr_net_weight)
{
  scale_info_t *ptr_scale;
  APP_ASSERT(ptr_net_weight);
  ptr_scale=seek_scale(id);
  if(ptr_scale)
  {
  *ptr_net_weight=ptr_scale->net_weight;
  return FORTUNA_TRUE;
  }
  return FORTUNA_FALSE;
}

/*设置净重值*/
fortuna_bool_t set_net_weight(uint8_t id,int32_t net_weight)
{
  scale_info_t *ptr_scale;
  ptr_scale=seek_scale(id);
  if(ptr_scale)
  {
  if(net_weight>=0x7FFF)
  {
    APP_LOG_ERROR("%d# 称正过载.\r\n",id); 
    ptr_scale->net_weight=SCALE_NET_WEIGHT_OVERLOAD_CODE;
  }
  else if(net_weight<=(-0x8000))
  {
    APP_LOG_ERROR("%d# 称负过载.\r\n",id); 
    ptr_scale->net_weight=SCALE_NET_WEIGHT_OVERLOAD_CODE;
  }
  else if(net_weight==SCALE_NET_WEIGHT_SPECIAL_NEGATIVE_VALUE && ptr_scale->timeout_cnt < SCALE_NET_WEIGHT_TIMEOUT_CNT)
  {   
   ptr_scale->net_weight=SCALE_NET_WEIGHT_SPECIAL_REPLACE_VALUE;
  }
  else
  {
    ptr_scale->net_weight= (int16_t)net_weight;
  }
  ptr_scale->timeout_cnt=0;
  return FORTUNA_TRUE;
  } 
  return FORTUNA_FALSE;
}

/*更新超时错误次数*/
static void scale_timeout_update(uint8_t id)
{
  scale_info_t *ptr_scale;
  ptr_scale=seek_scale(id);
  if(ptr_scale)
  {
  ptr_scale->timeout_cnt++;
  if(ptr_scale->timeout_cnt >= SCALE_NET_WEIGHT_TIMEOUT_CNT)
  {
   set_net_weight(id,SCALE_NET_WEIGHT_ERR_CODE); 
  }
  }
}
  
/*计算电子秤数量*/
void check_scale_cnt(uint8_t id,uint8_t *ptr_scale_start,uint8_t *ptr_scale_end)
{
 if(id==0)/*对所有的称*/
 {
  *ptr_scale_start=1;
  *ptr_scale_end=SCALES_CNT_MAX;
 }
 else
 {
  *ptr_scale_start=id; 
  *ptr_scale_end= *ptr_scale_start;  
 }
}


/*电子秤手动清零范围设置*/
fortuna_bool_t scale_manully_zero_range(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 (void)scale_param;
 for(uint8_t i=0;i<DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT;i++)
 {
 param[i]=SCALE_ZERO_RANGE_VALUE>>((DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤设置手动置零范围...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,DEVICE_MANUALLY_CLEAR_RANGE_REG_ADDR,DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤设置手动置零范围失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤设置手动置零范围成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}
/*电子秤清零*/
fortuna_bool_t scale_clear_zero(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_MANUALLY_CLEAR_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 (void)scale_param;
 for(uint8_t i=0;i<DEVICE_MANUALLY_CLEAR_REG_CNT;i++)
 {
 param[i]=SCALE_CLEAR_ZERO_VALUE>>((DEVICE_MANUALLY_CLEAR_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤清零...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,DEVICE_MANUALLY_CLEAR_REG_ADDR,DEVICE_MANUALLY_CLEAR_REG_CNT,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤清零失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤清零成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤去皮/设置皮重值*/
fortuna_bool_t scale_remove_tare(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_TARE_WEIGHT_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);

 for(uint8_t i=0;i<DEVICE_TARE_WEIGHT_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_TARE_WEIGHT_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤去皮...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,DEVICE_TARE_WEIGHT_REG_ADDR,DEVICE_TARE_WEIGHT_REG_CNT,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤去皮失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤去皮成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}


/*电子秤标定内码值*/
fortuna_bool_t scale_calibrate_code(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_ZERO_CODE_REG_CNT];
 uint16_t reg_addr,reg_cnt;
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 
 if(scale_param==0)
 {
 reg_addr=DEVICE_ZERO_CODE_REG_ADDR;
 reg_cnt=DEVICE_ZERO_CODE_REG_CNT;
 APP_LOG_DEBUG("收到0点内码值标定消息.\r\n");
 }
 else
 {
 reg_addr=DEVICE_SPAN_CODE_REG_ADDR;
 reg_cnt=DEVICE_SPAN_CODE_REG_CNT;
 APP_LOG_DEBUG("收到非零点内码值标定消息.\r\n");
 }
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_ZERO_CODE_REG_CNT;i++)
 {
 param[i]=SCALE_AUTO_CODE_VALUE>>((DEVICE_ZERO_CODE_REG_CNT-1-i)*16);
 }
 
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤内码值标定...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,reg_addr,reg_cnt,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤内码值标定失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤内码值标定成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}


/*电子秤标定测量值*/
fortuna_bool_t scale_calibrate_measurement(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_ZERO_MEASUREMENT_REG_CNT];
 uint16_t reg_addr,reg_cnt;
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 
 if(scale_param==0)
 {
 reg_addr=DEVICE_ZERO_MEASUREMENT_REG_ADDR;
 reg_cnt=DEVICE_ZERO_MEASUREMENT_REG_CNT;
 APP_LOG_DEBUG("收到0点测量值标定消息.\r\n");
 }
 else
 {
 reg_addr=DEVICE_SPAN_MEASUREMENT_REG_ADDR;
 reg_cnt=DEVICE_SPAN_MEASUREMENT_REG_CNT;
 APP_LOG_DEBUG("收到非0点测量值标定消息.\r\n");
 }
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_ZERO_MEASUREMENT_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_ZERO_MEASUREMENT_REG_CNT-1-i)*16);
 }
 
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤测量值标定...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,reg_addr,reg_cnt,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤测量值标定失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤测量值标定成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤重量校准*/
fortuna_bool_t scale_calibrate_weight(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_ZERO_CALIBRATE_REG_CNT];
 uint16_t reg_addr,reg_cnt;
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 
 if(scale_param==0)
 {
 reg_addr=DEVICE_ZERO_CALIBRATE_REG_ADDR;
 reg_cnt=DEVICE_ZERO_CALIBRATE_REG_CNT;
 APP_LOG_DEBUG("收到0点重量值标定消息.\r\n");
 }
 else
 {
 reg_addr=DEVICE_SPAN_CALIBRATE_REG_ADDR;
 reg_cnt=DEVICE_SPAN_CALIBRATE_REG_CNT;
 APP_LOG_DEBUG("收到重量值标定消息.\r\n");
 }
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_ZERO_CALIBRATE_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_ZERO_CALIBRATE_REG_CNT-1-i)*16);
 }
 
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤重量值标定...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,reg_addr,reg_cnt,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤重量值标定失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤重量值标定成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}


/*电子秤获取净重*/
fortuna_bool_t scale_obtain_net_weight(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t weight[DEVICE_NET_WEIGHT_REG_CNT];
 int32_t net_weight;
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 (void)scale_param;
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤获取净重...\r\n",id);
  err_code=modbus_poll_read_holding_registers(id,DEVICE_NET_WEIGHT_REG_ADDR,DEVICE_NET_WEIGHT_REG_CNT,weight);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  scale_timeout_update(id);
  APP_LOG_ERROR("%2d#电子秤获取净重失败.\r\n",id);
  }
  else
  {
  net_weight=(int32_t)uint32_big_decode_uint16(weight);
  set_net_weight(id,net_weight);
  APP_LOG_DEBUG("%2d#电子秤获取净重成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤获取固件版本*/
fortuna_bool_t scale_obtain_firmware_version(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t version[DEVICE_FIRMWARE_VERTION_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 (void)scale_param;
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤获取固件版本...\r\n",id);
  err_code=modbus_poll_read_holding_registers(id,DEVICE_FIRMWARE_VERTION_REG_ADDR,DEVICE_FIRMWARE_VERTION_REG_CNT,version);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤获取固件版本失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤获取固件版本成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤设置最大称重值*/
fortuna_bool_t scale_set_max_weight(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_MAX_WEIGHT_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_MAX_WEIGHT_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_MAX_WEIGHT_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤设置最大称重值...\r\n",id);
   err_code=modbus_poll_write_multiple_registers(id,DEVICE_MAX_WEIGHT_REG_ADDR,DEVICE_MAX_WEIGHT_REG_CNT,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤设置最大称重值失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤设置最大称重值成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤设置分度值*/
fortuna_bool_t scale_set_division(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_DIVISION_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_DIVISION_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_DIVISION_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤设置分度值...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,DEVICE_DIVISION_REG_ADDR,DEVICE_DIVISION_REG_CNT,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤设置分度值失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤设置分度值成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤设备锁*/
fortuna_bool_t scale_lock_operation(uint8_t id,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_LOCK_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 mb_poll_status_t err_code;
 /*计算电子秤数量*/
 check_scale_cnt(id,&scale_start,&scale_end);
 if(scale_param==SCALE_UNLOCK_VALUE)
 {
 APP_LOG_DEBUG("解锁操作.\r\n");
 }
 else
 {
 APP_LOG_DEBUG("上锁操作.\r\n");
 }
 for(uint8_t i=0;i<DEVICE_LOCK_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_LOCK_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(id=scale_start;id<=scale_end;id++)
 {
  APP_LOG_DEBUG("%2d#电子秤锁操作...\r\n",id);
  err_code=modbus_poll_write_multiple_registers(id,DEVICE_LOCK_REG_ADDR,DEVICE_LOCK_REG_CNT,param);
  if(err_code!=MODBUS_POLL_STATUS_SUCCESS)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤锁操作失败.\r\n",id);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤锁操作成功.\r\n",id);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}



