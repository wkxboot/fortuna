#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "ntc_3950.h"
#include "temperature_task.h"
#include "ABDK_ZNHG_ZK.h"
#include "adc.h"
#define APP_LOG_MODULE_NAME   "[temperature]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_OFF    
#include "app_log.h"
#include "app_error.h"

osThreadId temperature_task_hdl;
/*温度值*/
static int8_t temperature[TEMPERATURE_CNT];
/*温度ADC单次的取样值*/
static volatile uint16_t adc_sample[TEMPERATURE_CNT];
/*温度ADC取样的平均值*/
static uint16_t sample_average[TEMPERATURE_CNT];


static void update_temperature(uint8_t t_idx,int8_t t)
{
 temperature[t_idx]=t;
}
static void check_temperature()
{
 for(uint8_t i=0;i<TEMPERATURE_CNT;i++)
 {
  if(temperature[i] == NTC_ERROR_T_VALUE)
 {
  APP_LOG_ERROR("#%d 温度计断线或者短路错误.\r\n",i+1); 
 }
 }
}

int8_t get_temperature(uint8_t t_idx)
{
 if(t_idx > TEMPERATURE_CNT-1)
 {
  APP_LOG_ERROR("没有这个温度传感器-%d.\r\n",t_idx);
  return NTC_ERROR_T_VALUE;
 }
 return temperature[t_idx];
}
int8_t get_average_temperature()
{
  int8_t t=0,temp;
  /*有多个温度计 我们只获取平均值*/
  for(uint8_t i=0;i<TEMPERATURE_CNT;i++)
  {
  temp=get_temperature(i);
  if(temp==NTC_ERROR_T_VALUE)
  return TEMPERATURE_TASK_ERR_T_VALUE;
  t+=temp;
  }
 return t;
}


void temperature_task(void const * argument)
{
 uint32_t sample_cusum[TEMPERATURE_CNT]={0};/*取样的累加和*/
 uint32_t sample_time=0;/*取样的时间*/
 uint16_t sample_cnt=0;/*取样的次数*/
 APP_LOG_INFO("######温度任务开始.\r\n");

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
 /*检查温度是否有错误*/
 check_temperature();
 }
}
