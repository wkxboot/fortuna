#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "scales.h"
#include "display_led.h"
#include "display_task.h"
#include "switch_task.h"
#include "calibrate_memory_task.h"
#define APP_LOG_MODULE_NAME   "[cali_mem]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

/*校准显示缓存*/
dis_num_t calibrate_dis_buff[DISPLAY_LED_POS_CNT];

/*校准显示缓存任务*/
osThreadId calibrate_memory_task_hdl;


#define CALIBRATE_MEMORY_SCALE_MIN               1
#define CALIBRATE_MEMORY_SCALE_MAX               SCALES_CNT_MAX

#define CALIBRATE_MEMORY_DISPLAY_POS_MIN         0
#define CALIBRATE_MEMORY_DISPLAY_POS_MAX         (DISPLAY_LED_POS_CNT-1)

#define CALIBRATE_MEMORY_DISPLAY_NUM_MIN         0
#define CALIBRATE_MEMORY_DISPLAY_NUM_MAX         9

typedef struct
{
  uint8_t   idx;
  uint8_t   idx_min;
  uint8_t   idx_max;
}idx_range_t;

/*校准信息*/
typedef struct 
{
  idx_range_t  buff[DISPLAY_LED_POS_CNT];
  idx_range_t  pos;
  uint8_t      flash_code;
  uint8_t      flash_flag;/*最多8位数码管*/
  uint16_t     flash_time;
  uint16_t     cur_flash_time;
}calibrate_info_t;


/*校准信息变量*/
static calibrate_info_t calibrate;

/*校准信息变量初始化*/
static void calibrate_memory_task_init()
{
 /*显示位置范围0-5*/
 calibrate.pos.idx_min=CALIBRATE_MEMORY_DISPLAY_POS_MIN;
 calibrate.pos.idx_max=CALIBRATE_MEMORY_DISPLAY_POS_MAX;
 calibrate.pos.idx=CALIBRATE_MEMORY_DISPLAY_POS_MIN;
 calibrate.flash_code=DISPLAY_LED_NULL_NUM;
 calibrate.flash_time=CALIBRATE_MEMORY_TASK_FLASH_TIME;
 
 /*称号显示范围1-4*/
 calibrate.buff[0].idx_min=CALIBRATE_MEMORY_SCALE_MIN;
 calibrate.buff[0].idx_max=CALIBRATE_MEMORY_SCALE_MAX;
 calibrate.buff[0].idx=CALIBRATE_MEMORY_SCALE_MIN;
 
 /*数字显示范围0-9*/
 for(uint8_t i=1;i<DISPLAY_LED_POS_CNT;i++)
 {
 calibrate.buff[i].idx_min=CALIBRATE_MEMORY_DISPLAY_NUM_MIN;
 calibrate.buff[i].idx_max=CALIBRATE_MEMORY_DISPLAY_NUM_MAX;
 calibrate.buff[i].idx=CALIBRATE_MEMORY_DISPLAY_NUM_MIN;
 }
 /*显示缓存第一个表示称号 需要加上小数点*/
 for(uint8_t i=0;i<DISPLAY_LED_POS_CNT;i++)
 calibrate_dis_buff[i].dp=FORTUNA_FALSE;
 
 calibrate_dis_buff[0].dp=FORTUNA_TRUE;
}
/*外部使用*/
uint8_t get_calibrate_memory_calibrate_idx()
{
return calibrate.buff[0].idx;/*第一次显示位置是称号*/
}
uint32_t get_calibrate_memory_calibrate_weight()
{
 uint32_t weight=0;

 weight+=calibrate.buff[1].idx*10000;
 weight+=calibrate.buff[2].idx*1000;
 weight+=calibrate.buff[3].idx*100;
 weight+=calibrate.buff[4].idx*10;
 weight+=calibrate.buff[5].idx;

 return weight; 
}



/*校准显示缓存任务*/
void calibrate_memory_task(void const * argument)
{
 osEvent signal;
 /*初始化状态*/
 calibrate_memory_task_init();
 APP_LOG_INFO("######校准显示缓存任务开始.\r\n");
 while(1)
 {
 signal=osSignalWait(CALIBRATE_MEMORY_TASK_ALL_SIGNALS,CALIBRATE_MEMORY_TASK_INTERVAL);
 if(signal.status==osEventSignal)
 {
  /*开始校准 闪烁对应的位*/
  if(signal.value.signals & CALIBRATE_MEMORY_TASK_CALIBRATE_START_SIGNAL)
  {
  /*设置进入校准时默认闪烁位置*/
  calibrate.pos.idx=calibrate.pos.idx_min;
  /*更新闪烁标志*/
  calibrate.flash_flag=1<<(calibrate.pos.idx);
  calibrate.cur_flash_time=0;
  }
  if(signal.value.signals & CALIBRATE_MEMORY_TASK_UPDATE_POS_SIGNAL)
  {
   if(calibrate.pos.idx>=calibrate.pos.idx_max)
   calibrate.pos.idx=calibrate.pos.idx_min;
   else
   calibrate.pos.idx++;
   
  /*更新闪烁标志*/
  calibrate.flash_flag=1<<(calibrate.pos.idx);
  calibrate.cur_flash_time=0;
  }
  if(signal.value.signals & CALIBRATE_MEMORY_TASK_INCREASE_SIGNAL)
  {
   if(calibrate.buff[calibrate.pos.idx].idx>=calibrate.buff[calibrate.pos.idx].idx_max)
   calibrate.buff[calibrate.pos.idx].idx=calibrate.buff[calibrate.pos.idx].idx_min;
   else
   calibrate.buff[calibrate.pos.idx].idx++;   
  }
  if(signal.value.signals & CALIBRATE_MEMORY_TASK_DECREASE_SIGNAL)
  {
   if(calibrate.buff[calibrate.pos.idx].idx<=calibrate.buff[calibrate.pos.idx].idx_min)
   calibrate.buff[calibrate.pos.idx].idx=calibrate.buff[calibrate.pos.idx].idx_max;
   else
   calibrate.buff[calibrate.pos.idx].idx--;   
  }
  if(signal.value.signals & CALIBRATE_MEMORY_TASK_CALIBRATE_OK_SIGNAL)
  {
  /*更新闪烁标志 如果校准成功就不在闪烁 全亮*/
  calibrate.flash_flag=0; 
  calibrate.cur_flash_time=0;
  }
  if(signal.value.signals & CALIBRATE_MEMORY_TASK_CALIBRATE_ERR_SIGNAL)
  {
  /*更新闪烁标志 如果校准失败就同步全闪烁*/
  calibrate_dis_buff[calibrate.pos.idx].num=calibrate.buff[calibrate.pos.idx].idx; 
  calibrate.flash_flag=0xff;
  calibrate.cur_flash_time=0;
  
  }
 }
  /*更新显存的数字*/
 for(uint8_t i=0;i<DISPLAY_LED_POS_CNT;i++)
 {
  if(calibrate.flash_flag & (1<<i))/*这一位正在闪烁 取空字符*/
  {
  if(calibrate.cur_flash_time >= calibrate.flash_time)/*到达闪烁时间内*/
  {
    if(calibrate_dis_buff[i].num==calibrate.flash_code)
     calibrate_dis_buff[i].num=calibrate.buff[i].idx;
    else
     calibrate_dis_buff[i].num=calibrate.flash_code; 
  }
  }
  else
  calibrate_dis_buff[i].num=calibrate.buff[i].idx;
 }
 if(calibrate.cur_flash_time >= calibrate.flash_time)/*到达闪烁时间内*/   
  calibrate.cur_flash_time=0;
 
/*更新显示状态*/ 
 if(calibrate.flash_flag!=0)
 calibrate.cur_flash_time+=CALIBRATE_MEMORY_TASK_INTERVAL;
 }
}