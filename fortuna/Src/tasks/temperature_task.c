#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_AHG081_ZK.h"
#include "ntc_3950.h"
#include "temperature_task.h"
#include "adc.h"
#define APP_LOG_MODULE_NAME   "[temperature]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"

osThreadId temperature_task_hdl;

/*温度ADC单次的取样值*/
static volatile uint16_t adc_sample[TEMPERATURE_CNT];
/*温度ADC取样的平均值*/
static uint16_t sample_average[TEMPERATURE_CNT];

/*为了温度平滑变化 需要温度值在一个变化方向上保持一定时间 才可以相反方向变化*/
/*温度变化方向*/
typedef enum
{
  T_NO_DIRECTION=0,
  T_INCREASE=1,
  T_DECREASE=2
}t_direction_t;


/*温度数值控制信息*/
typedef struct
{
  t_direction_t dir;
  int8_t temperature;
  uint32_t hold_time;
}temperature_ctl_t;

/*一个变化方向上的保持时间最小值 10s*/
#define           T_HOLD_TIME                10*1000

/*温度值*/
static int8_t temperature[TEMPERATURE_CNT];
static temperature_ctl_t average_temperature;


static void temperature_init()
{
 average_temperature.dir=T_NO_DIRECTION;
 average_temperature.temperature=0;
 average_temperature.hold_time=T_HOLD_TIME;
}
static void update_temperature(uint8_t t_idx,int8_t t)
{
if(t==NTC_ERROR_T_VALUE)
{
temperature[t_idx]=TEMPERATURE_TASK_ERR_T_VALUE;
APP_LOG_ERROR("%d#温度计断线或者短路或者超量程错误.\r\n",t_idx+1);
return;
}
temperature[t_idx]=t;
}


static void update_average_temperature()
{
   int16_t t=0,temp=0;
  /*有多个温度计 我们只获取平均值*/
  for(uint8_t i=0;i<TEMPERATURE_CNT;i++)
  {
  temp=get_temperature(i);
  if(temp==TEMPERATURE_TASK_ERR_T_VALUE)
  {
  average_temperature.temperature= TEMPERATURE_TASK_ERR_T_VALUE;
  return;
  }
  t+=temp;
  }
  t/=TEMPERATURE_CNT;
 /*如果上一次是错误状态 就直接赋值*/
 if(average_temperature.temperature==TEMPERATURE_TASK_ERR_T_VALUE)
 {
  average_temperature.temperature=t;
  average_temperature.hold_time=T_HOLD_TIME;/*方向随时可变*/
  return;
 }
 
/*如果新获取的温度大于或者小于缓存的温度值 或者实际保持时间大于最小保持时间*/
if((t > average_temperature.temperature && average_temperature.dir == T_INCREASE) ||\
   (t < average_temperature.temperature && average_temperature.dir == T_DECREASE) ||\
   (average_temperature.hold_time >= T_HOLD_TIME && t != average_temperature.temperature))
{
  if(t > average_temperature.temperature)
  average_temperature.dir=T_INCREASE;
  else
  average_temperature.dir=T_DECREASE;  
  
   /*当前温度是增加的*/
  average_temperature.temperature=t;
  average_temperature.hold_time=0;  
}
else
average_temperature.hold_time+=TEMPERATURE_SAMPLE_TIME;  

}

int8_t get_temperature(uint8_t t_idx)
{
 if(t_idx > TEMPERATURE_CNT-1)
 {
  APP_LOG_ERROR("没有这个温度传感器-%d.\r\n",t_idx);
  return TEMPERATURE_TASK_ERR_T_VALUE;
 }
 return temperature[t_idx];
}
int8_t get_average_temperature()
{
return average_temperature.temperature;
}


void temperature_task(void const * argument)
{
 uint32_t sample_cusum[TEMPERATURE_CNT]={0};/*取样的累加和*/
 uint32_t sample_time=0;/*取样的时间*/
 uint16_t sample_cnt=0;/*取样的次数*/
 APP_LOG_INFO("@温度任务开始.\r\n");
/*温度信息初始化*/
 temperature_init();
 while(1)
 {
 while(sample_time<TEMPERATURE_SAMPLE_TIME)
 {
 /*adc 开始取样*/
 HAL_ADC_Start_DMA(&hadc3,(uint32_t*)adc_sample,TEMPERATURE_CNT);
 /*等待DMA取样结束*/
 osDelay(TEMPERATURE_MONITOR_INTERVAL);
 /*开始计算取样累加值*/
 for(uint8_t i=0;i<TEMPERATURE_CNT;i++)
 {
 sample_cusum[i]+=adc_sample[i];  
 }
 sample_time+=TEMPERATURE_MONITOR_INTERVAL;
 sample_cnt++;/*更新取样次数，为平均值准备*/
 }
 APP_LOG_DEBUG("温度ADC取样时间：%d ms，取样次数：%d.\r\n",sample_time,sample_cnt);
 for(uint8_t i=0;i<TEMPERATURE_CNT;i++)
 {
 sample_average[i]=sample_cusum[i]/sample_cnt; 
 APP_LOG_DEBUG("累加值[%d]：%d.\r\n",i,sample_cusum[i]);
 APP_LOG_DEBUG("平均值[%d]：%d.\r\n",i,sample_average[i]);
 } 
 sample_time=0;
 sample_cnt=0; 
 /*不需要连续总和计算，而是下次重新计算*/
 for(uint8_t i=0;i<TEMPERATURE_CNT;i++)
 {
 sample_cusum[i]=0;
 }
 /*计算温度值*/
 for(uint8_t i=0;i<TEMPERATURE_CNT;i++)
 {
 update_temperature(i,ntc_3950_get_t(sample_average[i]));
 }
 /*计算更新平均值*/
 update_average_temperature();
 }
}
