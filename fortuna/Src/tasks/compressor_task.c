#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "temperature_task.h"
#include "compressor_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[compressor]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId compressor_task_hdl;

void compressor_task(void const * argument)
{
 static osEvent signal;
 static int16_t cur_t;
 static uint32_t work_time=0,stop_time=COMPRESSOR_TASK_STOP_MIN_TIME;/*准备随时可以开机*/;
 APP_LOG_INFO("######压缩机任务开始.\r\n"); 
 /*开机时先关闭压缩机*/
 APP_LOG_DEBUG("关闭压缩机！\r\n");
 BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_OFF);
 osDelay(1000);/*等待1s*/
 
 while(1)
 {
/*温度错误时处理goto handle*/
compressor_start:
 signal=osSignalWait(COMPRESSOR_TASK_ALL_SIGNALS,COMPRESSOR_TASK_INTERVAL);
 
 /*计算工作时间长和停机时长*/
 if(BSP_get_compressor_pwr_state()==COMPRESSOR_PWR_STATE_ON)
 {
 work_time+=COMPRESSOR_TASK_INTERVAL;
 if(stop_time!=0)
 stop_time=0;
 }
 else
 {
 stop_time +=COMPRESSOR_TASK_INTERVAL;
 if(work_time!=0)
 work_time=0;
 }
 /*处理信号 调试信号我们不关注工作和停机时长，都是强制执行*/
 if(signal.status==osEventSignal)
 {
  /*需要压缩机强制执行前的状态*/
  bsp_state_t prv_pwr_state;
  prv_pwr_state=BSP_get_compressor_pwr_state();
  APP_LOG_DEBUG("压缩机任务收到信号.\r\n");
 if(signal.value.signals & COMPRESSOR_TASK_PWR_ON_SIGNAL)
 {
  APP_LOG_DEBUG("打开压缩机！\r\n");
  BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_ON);
 }
 else
 {
  APP_LOG_DEBUG("关闭压缩机！\r\n");
  BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_OFF);
 }
 /*强制执行一段时间*/
 osDelay(COMPRESSOR_TASK_DEBUG_WORK_TIME);
 /*然后需要还原强制执行前的状态*/
 if(prv_pwr_state==COMPRESSOR_PWR_STATE_ON)
 {
  APP_LOG_DEBUG("还原以前的状态.打开压缩机！\r\n");
  BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_ON);
 }
 else
 {
  APP_LOG_DEBUG("还原以前的状态.关闭压缩机！\r\n");
  BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_OFF);
 }
 }
 /*获取温度计的平均值*/
 cur_t=get_average_temperature();
 /*当出现温度计错误时，关闭所有压缩机*/
 if(cur_t==TEMPERATURE_TASK_ERR_T_VALUE)
 {
   if(BSP_get_compressor_pwr_state()!=COMPRESSOR_PWR_STATE_OFF)
   {
   APP_LOG_DEBUG("关闭压缩机！\r\n");
   BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_OFF);
   stop_time=COMPRESSOR_TASK_STOP_MIN_TIME;/*准备随时可以开机*/
   }
  goto compressor_start;
 }
 
 /*检查压缩机是否在工作温度范围 并且满足工作时间要求*/
 if(cur_t>COMPRESSOR_TASK_T_MAX && BSP_get_compressor_pwr_state()==COMPRESSOR_PWR_STATE_OFF && stop_time >=COMPRESSOR_TASK_STOP_MIN_TIME)
 {
  APP_LOG_DEBUG("打开压缩机！\r\n");
  BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_ON);
 }
 else if((cur_t < COMPRESSOR_TASK_T_MIN && BSP_get_compressor_pwr_state()==COMPRESSOR_PWR_STATE_ON) || work_time >=COMPRESSOR_TASK_WORK_MAX_TIME)
 {
  APP_LOG_DEBUG("关闭压缩机！\r\n");
  BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_OFF);
 }
 }
}
