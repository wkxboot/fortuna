#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "scales.h"
#include "digit_led.h"
#include "display_task.h"
#include "switch_task.h"
#include "weight_cache_task.h"
#define APP_LOG_MODULE_NAME   "[w_cache]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"


#define WEIGHT_CACHE_IDX_MIN              1
#define WEIGHT_CACHE_IDX_MAX              SCALES_CNT_MAX

/*重量层数缓存结构*/
typedef struct
{
 uint8_t  idx; 
 uint8_t  idx_min;
 uint8_t  idx_max;
}weight_info_t;

/*重量层数缓存变量*/
static weight_info_t weight_info=
{
.idx_min=WEIGHT_CACHE_IDX_MIN,
.idx_max=WEIGHT_CACHE_IDX_MAX,
.idx=WEIGHT_CACHE_IDX_MIN
};

/*重量显示缓存*/
dis_num_t w_dis_buff[DIGIT_LED_POS_CNT];

/*重量显示缓存任务*/
osThreadId weight_cache_task_hdl;

uint8_t get_weight_cache_idx()
{
 return weight_info.idx;
}


/*重量显示缓存任务*/
void weight_cache_task(void const * argument)
{
 osEvent signal;
 int16_t net_weight;
 APP_LOG_INFO("@重量显示缓存任务开始.\r\n");
 while(1)
 {
 signal=osSignalWait(WEIGHT_CACHE_TASK_ALL_SIGNALS,WEIGHT_CACHE_TASK_INTERVAL);
 if(signal.status==osEventSignal)
 {
  /*更新显示重量的层数*/
  if(signal.value.signals & WEIGHT_CACHE_TASK_UPDATE_IDX_SIGNAL)
  {
   APP_LOG_DEBUG("重量显示缓存任务收到更新重量层数信号.\r\n");
   if(weight_info.idx >= weight_info.idx_max)
   weight_info.idx=weight_info.idx_min;
   else
   weight_info.idx++;
  }
 }
 get_net_weight(weight_info.idx,&net_weight);

 
 /*1显示层数*/
 w_dis_buff[0].num=weight_info.idx;
 w_dis_buff[0].dp=APP_TRUE;
 /*2-6显示重量*/
 if(net_weight==SCALE_NET_WEIGHT_ERR_CODE)
 {
   APP_LOG_ERROR("%d#称重量值错误.\r\n",weight_info.idx);
   for(uint8_t i=1;i<DIGIT_LED_POS_CNT;i++)
   w_dis_buff[i].num=0x0f;/*重量值错误显示FFFFF*/
 }
 else if(net_weight == SCALE_NET_WEIGHT_OVERLOAD_CODE)
 {
   APP_LOG_ERROR("%d#称重量值超量程.\r\n",weight_info.idx);
   for(uint8_t i=1;i<DIGIT_LED_POS_CNT;i++)
   w_dis_buff[i].num=DIGIT_LED_NEGATIVE_NUM;
 }
 else if(net_weight<0)
 {
 net_weight*=-1;
 w_dis_buff[1].num=DIGIT_LED_NULL_NUM;
 w_dis_buff[2].num=net_weight/1000;
 net_weight=net_weight%1000;
 w_dis_buff[3].num=net_weight/100; 
 net_weight=net_weight%100;
 w_dis_buff[4].num=net_weight/10;
 net_weight=net_weight%10;
 w_dis_buff[5].num=net_weight;
 
 for(uint8_t i=2;i<DIGIT_LED_POS_CNT;i++)
 {
  if(w_dis_buff[i].num==0)/*消0显示*/
  {
  w_dis_buff[i].num=DIGIT_LED_NULL_NUM;
  }
  else
  {
   /*证明最左边0已经找到 退出*/
   w_dis_buff[i-1].num=DIGIT_LED_NEGATIVE_NUM;
   break; 
  }   
 }
 }
 else if(net_weight>=0)
 {
 w_dis_buff[1].num=net_weight/10000;
 net_weight=net_weight%10000;
 w_dis_buff[2].num=net_weight/1000;
 net_weight=net_weight%1000;
 w_dis_buff[3].num=net_weight/100;
 net_weight=net_weight%100;
 w_dis_buff[4].num=net_weight/10;
 net_weight=net_weight%10;
 w_dis_buff[5].num=net_weight;
 
 for(uint8_t i=1;i<DIGIT_LED_POS_CNT-1;i++)
 {
  if(w_dis_buff[i].num==0)/*消0显示*/
  {
  w_dis_buff[i].num=DIGIT_LED_NULL_NUM;
  }
  else
  {
   /*证明最左边0已经找到 退出*/
   break; 
  }   
 }
 }
 
 }
}