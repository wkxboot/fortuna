#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "scales.h"
#include "display_led.h"
#include "display_task.h"
#include "switch_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[switch]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId switch_task_hdl;

typedef void (*ptr_sw_func)(void);

/*每一种按键的模式数量 即正常状态下和进入校准状态2种*/
#define  SWITCH_MODE_CNT         2

typedef enum
{
  SWITCH_MODE_NORMAL=0,
  SWITCH_MODE_CALIBRATE
}switch_mode_t;

typedef struct ___switch_mode
{
uint8_t             idx;/*模式标号*/
const uint8_t       idx_max;
const switch_mode_t mode[SWITCH_MODE_CNT];
}switch_mode_info_t;

typedef struct __switch
{ 
 bsp_state_t pre_state;
 bsp_state_t cur_state;
 uint32_t    hold_time;/*此状态保持的时间*/
 uint32_t    short_press_time;/*短按有效时间*/
 uint32_t    long_press_time;/*长按有效时间*/
 ptr_sw_func short_press[SWITCH_MODE_CNT];
 ptr_sw_func long_press[SWITCH_MODE_CNT];
 switch_mode_info_t *ptr_sw_info;
}switch_state_t;

/*按键短按和长按有效时间*/
#define  SWITCH_SHORT_PRESS_TIME          50
#define  SWITCH_LONG_PRESS_TIME           2000

/*温度显示缓存*/
static dis_num_t t_dis_buff[DISPLAY_LED_POS_CNT];
/*重量显示缓存*/
static dis_num_t w_dis_buff[DISPLAY_LED_POS_CNT];
/*校准时的显示缓存*/
static dis_num_t calibrate_dis_buff[DISPLAY_LED_POS_CNT];
/*显示任务缓存指针*/
extern dis_num_t *ptr_buff;

/*当前按键处在的模式信息*/
switch_mode_info_t switch_info=
{
 /*按键模式信息初始化*/
.idx=0,
.idx_max=SWITCH_MODE_CNT,
.mode[0]=SWITCH_MODE_NORMAL,
.mode[1]=SWITCH_MODE_CALIBRATE
};
/*所有按键对象*/
static switch_state_t wt_sw,w_sw,calibrate_sw,zero_sw,tare_sw;
/*
 *1.重量温度切换键
 *2.重量切换键
 *3.校准键（长按2秒进入。校准时短按循环位移，长按为OK）
 *4.去皮（校准时为+）
 *5.清零（校准时为-）
*/

/*正常状态下重量和温度切换按键短按和长按功能函数*/
static void wt_sw_short_press_normal()
{
 if(ptr_buff==w_dis_buff)
 {
   ptr_buff=t_dis_buff;
   APP_LOG_DEBUG("切换成温度显示.\r\n");
 }
 else
 {
   ptr_buff=w_dis_buff;
   APP_LOG_DEBUG("切换成重量显示.\r\n");
 }
}
static void wt_sw_long_press_normal()
{
  APP_LOG_DEBUG("重量温度切换按键无长按功能.等效为短按.\r\n");
  wt_sw_short_press_normal();
}
/*校准状态下重量和温度切换按键短按和长按功能函数*/
static void wt_sw_short_press_calibrate()
{
 APP_LOG_DEBUG("重量温度切换按键校准状态下无短按功能.\r\n");
}
static void wt_sw_long_press_calibrate()
{
 APP_LOG_DEBUG("重量温度切换按键校准状态下无长按功能.\r\n");
}
/*正常状态下重量选择按键短按和长按功能函数*/
static void w_sw_short_press_normal()
{
 APP_LOG_DEBUG("向重量显存任务发送更新显示的对象.\r\n");
 /*向重量显存任务发送更新显示的对象*/
 osSignalSet(weight_memory_task,WEIGHT_MEMORY_TASK_UPDATE_IDX);
}
static void w_sw_long_press_normal()
{
  APP_LOG_DEBUG("重量选择按键无长按功能.等效为短按.\r\n");
  w_sw_short_press_normal();
}
/*校准状态下重量选择按键短按和长按功能函数*/
static void w_sw_short_press_calibrate()
{
 APP_LOG_DEBUG("重量选择按键校准状态下无短按功能.\r\n"); 
}
static void w_sw_long_press_calibrate()
{
 APP_LOG_DEBUG("重量选择按键校准状态下无长按功能.\r\n"); 
}
/*正常状态下校准按键短按和长按功能函数*/
static void calibrate_sw_short_press_normal()
{
 APP_LOG_DEBUG("校准按键正常状态下无短按功能.\r\n");  
}
static void calibrate_sw_long_press_normal()
{
 APP_LOG_DEBUG("校准按键切换模式...\r\n");  
 calibrate_sw.ptr_sw_info->idx++;
 if(calibrate_sw.ptr_sw_info->idx >=calibrate_sw.ptr_sw_info->idx_max)
 calibrate_sw.ptr_sw_info->idx=0;
 /*更新模式对应的显示缓存指针*/
 if(calibrate_sw.ptr_sw_info->mode[calibrate_sw.ptr_sw_info->idx]==SWITCH_MODE_NORMAL)
 {
 /*从校准模式退出时 需要等待校准完成 just等待2秒*/
 osDelay(SWITCH_TASK_CALIBRATE_EXIT_WAIT_TIME);
 ptr_buff=weight_dis_buff;
 APP_LOG_DEBUG("切换为正常模式.\r\n");
 }
 else
 {
 ptr_buff==calibrate_dis_buff;
 APP_LOG_DEBUG("切换为校准模式.\r\n");
 }
}
/*校准状态下校准按键短按和长按功能函数*/
static void calibrate_sw_short_press_calibrate()
{
/*向校准显存任务发送更新数字位置信号*/
APP_LOG_DEBUG("向校准显存任务发送更新数字位置信号.\r\n");
osSignalSet(calibrate_memory_task_hdl,CALIBRATE_MEMORY_TASK_UPDATE_POS_SIGNAL);  
}
static void calibrate_sw_long_press_calibrate()
{
 /*校准模式下校准按键长按功能等效为短按*/
 APP_LOG_DEBUG("校准状态下位选择按键无长按功能.等效为短按.\r\n");  
 calibrate_sw_short_press_calibrate(); 
}
/*正常状态下去皮短按和长按功能函数*/
static void tare_sw_short_press_normal()
{
 /*向电子秤功能任务发送去皮信号*/
 uint8_t w_idx;
 scale_msg_t msg;
 w_idx=get_weight_memory_idx();
 msg.type=SCALE_FUNC_TASK_CLEAR_TARE_WEIGHT_MSG;
 msg.scale=w_idx;
 APP_LOG_DEBUG("去皮按键.\r\n");
 APP_LOG_DEBUG("向电子秤功能任务发送去皮指令.\r\n");
 osMessagePut(scale_func_task_msg_q_id,*(uint32_t*)&msg,0);/*立即执行返回*/
}
static void tare_sw_long_press_normal()
{
  APP_LOG_DEBUG("正常状态下去皮按键无长按功能.等效为短按.\r\n");
  tare_sw_short_press_normal();
}
/*校准状态下去皮按键短按和长按功能函数(+号按键)*/
static void tare_sw_short_press_calibrate()
{
 APP_LOG_DEBUG("+按键短按.\r\n");
 APP_LOG_DEBUG("向校准显存任务发送增加数字值信号.\r\n");
 osSignalSet(calibrate_memory_task_hdl,CALIBRATE_MEMORY_TASK_INCREASE_SIGNAL);
}
static void tare_sw_long_press_calibrate()
{
 APP_LOG_DEBUG("校准状态下+按键无长按功能.等效为短按.\r\n");
 tare_sw_short_press_calibrate();  
}
/*正常状态下清零短按和长按功能函数*/
static void zero_sw_short_press_normal()
{
 /*向电子秤功能任务发送清零信号*/
 uint8_t w_idx;
 scale_msg_t msg;
 w_idx=get_weight_memory_idx();
 msg.type=SCALE_FUNC_TASK_CLEAR_ZERO_WEIGHT_MSG;
 msg.scale=w_idx;
 APP_LOG_DEBUG("清零按键.\r\n");
 APP_LOG_DEBUG("向电子秤功能任务发送清零指令.\r\n");
 osMessagePut(scale_func_task_msg_q_id,*(uint32_t*)&msg,0);/*立即执行返回*/ 
}
static void zero_sw_long_press_normal()
{
 APP_LOG_DEBUG("正常状态下清零按键无长按功能.等效为短按.\r\n");
 zero_sw_short_press_normal(); 
}
/*校准状态下清零按键短按和长按功能函数(-号按键)*/
static void zero_sw_short_press_calibrate()
{
 APP_LOG_DEBUG("-按键短按.\r\n");
 APP_LOG_DEBUG("向校准显存任务发送减少数字值信号.\r\n");
 osSignalSet(calibrate_memory_task_hdl,CALIBRATE_MEMORY_TASK_DECREASE_SIGNAL); 
}
static void zero_sw_long_press_calibrate()
{
 APP_LOG_DEBUG("校准状态下-按键无长按功能.等效为短按.\r\n");
 zero_sw_short_press_calibrate();   
}

static void switch_init()
{
/*重量温度切换按键*/
wt_sw.short_press_time=SWITCH_SHORT_PRESS_TIME;
wt_sw.long_press_time=SWITCH_LONG_PRESS_TIME;
wt_sw.short_press[0]=wt_sw_short_press_normal;
wt_sw.long_press[0]=wt_sw_long_press_normal;
wt_sw.short_press[1]=wt_sw_short_press_calibrate;
wt_sw.long_press[1]=wt_sw_long_press_calibrate;
wt_sw.ptr_sw_info=&switch_info;
/*重量层数选择按键*/
w_sw.short_press_time=SWITCH_SHORT_PRESS_TIME;
w_sw.long_press_time=SWITCH_LONG_PRESS_TIME;
w_sw.short_press[0]=w_sw_short_press_normal;
w_sw.long_press[0]=w_sw_long_press_normal;
w_sw.short_press[1]=w_sw_short_press_calibrate;
w_sw.long_press[1]=w_sw_long_press_calibrate;  
w_sw.ptr_sw_info=&switch_info;
/*校准按键*/  
calibrate_sw.short_press_time=SWITCH_SHORT_PRESS_TIME;
calibrate_sw.long_press_time=SWITCH_LONG_PRESS_TIME;
calibrate_sw.short_press[0]=calibrate_sw_short_press_normal; 
calibrate_sw.long_press[0]=calibrate_sw_long_press_normal;
calibrate_sw.short_press[1]=calibrate_sw_short_press_calibrate;
calibrate_sw.long_press[1]=calibrate_sw_long_press_calibrate; 
calibrate_sw.ptr_sw_info=&switch_info;
/*去皮按键*/
tare_sw.short_press_time=SWITCH_SHORT_PRESS_TIME;
tare_sw.long_press_time=SWITCH_LONG_PRESS_TIME;
tare_sw.short_press[0]=tare_sw_short_press_normal; 
tare_sw.long_press[0]=tare_sw_long_press_normal;
tare_sw.short_press[1]=tare_sw_short_press_calibrate;
tare_sw.long_press[1]=tare_sw_long_press_calibrate;
tare_sw.ptr_sw_info=&switch_info;
/*清零按键*/
zero_sw.short_press_time=SWITCH_SHORT_PRESS_TIME;
zero_sw.long_press_time=SWITCH_LONG_PRESS_TIME;
zero_sw.short_press[0]=zero_sw_short_press_normal; 
zero_sw.long_press[0]=zero_sw_long_press_normal;
zero_sw.short_press[1]=zero_sw_short_press_calibrate;
zero_sw.long_press[1]=zero_sw_long_press_calibrate; 
zero_sw.ptr_sw_info=&switch_info;
/*默认显示重量*/
ptr_buff=w_dis_buff;

APP_LOG_DEBUG("所有按键初始化完毕.\r\n");

}

/*按键扫描任务*/
void switch_task(void const * argument)
{
 bsp_state_t state;
 APP_LOG_INFO("######按键状态任务开始.\r\n");
 /*初始化按键*/
 switch_init();
  /*从F-0依次显示，检测显示是否正常*/
 APP_LOG_DEBUG("初始化数码管显示...\r\n"); 
 for(uint8_t i=0x0f;i>0;i--)
 {
  for(uint8_t j=0;j<DISPLAY_LED_POS_CNT;j++)
  {
  ptr_buff[j].num=i;  
  ptr_buff[j].dp=FORTUNA_TRUE;
  }
  osDelay(INIT_DISPLAY_HOLD_ON_TIME);
 }
 APP_LOG_DEBUG("初始化数码管显示完毕.\r\n");
 while(1)
 {   
 osDelay(SWITCH_TASK_INTERVAL);
 /*查看重量温度切换按键*/
 wt_sw.cur_state=BSP_get_wt_sw_state();
 if(wt_sw.pre_state!=wt_sw.cur_state)
 {
  if(wt_sw.cur_state==SW_STATE_PRESS)/*因为有长短按压时间 所以按键定义为释放时有效*/ 
  {
   wt_sw.pre_state=wt_sw.cur_state;
   wt_sw.hold_time=0;
  }
  else/*释放*/
  {
   if(wt_sw.hold_time>=wt_sw.long_press_time)/*长按释放*/
   {
    wt_sw.long_press[wt_sw.ptr_sw_info.idx](); 
   }
   else if(wt_sw.hold_time>=wt_sw.short_press_time)/*短按释放*/
   {
    wt_sw.short_press[wt_sw.ptr_sw_info.idx]();   
   }
   wt_sw.hold_time=0;
  }
 }
 else
 {
  wt_sw.hold_time+=SWITCH_TASK_INTERVAL;/*更新保持时间 去抖动*/
 }
 
 }
}