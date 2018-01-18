#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "mb_m.h"
#include "scales.h"
#include "scale_func_task.h"
#include "display_led.h"
#include "display_task.h"
#include "switch_task.h"
#include "weight_memory_task.h"
#include "temperature_memory_task.h"
#include "calibrate_memory_task.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[switch]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

/*按键扫描任务*/
osThreadId switch_task_hdl;

/*按键数量定义*/
#define  SWITCH_CNT                               5
#define  WT_SWITCH_IDX                            0
#define  W_SWITCH_IDX                             1
#define  CALIBRATE_SWITCH_IDX                     2
#define  TARE_SWITCH_IDX                          3
#define  ZERO_SWITCH_IDX                          4

/*按键短按和长按有效时间*/
#define  SWITCH_SHORT_PRESS_TIME                  50
#define  SWITCH_LONG_PRESS_TIME                   2000

typedef void (*ptr_sw_func)(void);

/*每一种按键的模式数量 即正常状态下和进入校准状态2种*/
#define  SWITCH_MODE_CNT                         2

typedef enum
{
  SWITCH_MODE_NORMAL=0,
  SWITCH_MODE_CALIBRATE
}switch_mode_t;

/*按键模式信息*/
typedef struct __switch_mode
{
uint8_t             idx;/*模式标号*/
const uint8_t       idx_max;
const switch_mode_t mode[SWITCH_MODE_CNT];
const dis_num_t      *ptr_dis_buff[SWITCH_MODE_CNT];
}switch_mode_info_t;

/*按键状态和处理*/
typedef struct __switch
{ 
 bsp_state_t pre_state;
 bsp_state_t cur_state;
 uint32_t    hold_time;/*此状态保持的时间*/
 uint32_t    short_press_time;/*短按有效时间*/
 uint32_t    long_press_time;/*长按有效时间*/
 ptr_sw_func short_press[SWITCH_MODE_CNT];
 ptr_sw_func long_press[SWITCH_MODE_CNT];
}switch_state_t;
/*初始化显示缓存*/
static dis_num_t init_dis_buff[DISPLAY_LED_POS_CNT];

/*显示任务缓存指针*/
extern const dis_num_t *ptr_buff;

/*当前按键处在的模式信息*/
switch_mode_info_t switch_info=
{
 /*按键模式信息初始化*/
.idx=0,
.idx_max=SWITCH_MODE_CNT-1,
.mode[0]=SWITCH_MODE_NORMAL,
.mode[1]=SWITCH_MODE_CALIBRATE,
.ptr_dis_buff[0]=w_dis_buff,
.ptr_dis_buff[1]=calibrate_dis_buff
};
/*所有按键对象*/
static switch_state_t sw[SWITCH_CNT];
/*
 *1.重量温度切换键
 *2.重量切换键(校准时为取消校准)
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
 osSignalSet(weight_memory_task_hdl,WEIGHT_MEMORY_TASK_UPDATE_IDX_SIGNAL);
}
static void w_sw_long_press_normal()
{
  APP_LOG_DEBUG("重量选择按键无长按功能.等效为短按.\r\n");
  w_sw_short_press_normal();
}
/*校准状态下重量选择按键短按和长按功能函数*/
static void w_sw_short_press_calibrate()
{
 APP_LOG_DEBUG("重量选择按键校准状态下短按.放弃校准.\r\n"); 
 if(switch_info.idx >=switch_info.idx_max)
  switch_info.idx=0;
 else
  switch_info.idx++;
 /*更新模式对应的显示缓存指针*/
  ptr_buff=switch_info.ptr_dis_buff[switch_info.idx];
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
 uint8_t  calibrate_idx; 
 uint32_t calibrate_w;
 eMBMasterReqErrCode err_code;
 uint16_t param[2];
 uint16_t reg_addr,reg_cnt;
 
 if(switch_info.idx >=switch_info.idx_max)
  switch_info.idx=0;
 else
  switch_info.idx++;

 if(switch_info.mode[switch_info.idx]==SWITCH_MODE_NORMAL)
 {
  /*从校准模式退出时 需要等待校准完成*/
  calibrate_idx=get_calibrate_memory_calibrate_idx();
  calibrate_w =get_calibrate_memory_calibrate_weight();
  param[0]=calibrate_w>>16;
  param[1]=calibrate_w&0xffff;
  if(calibrate_w==0)
  {
  reg_addr=DEVICE_ZERO_CALIBRATE_REG_ADDR;
  reg_cnt=DEVICE_ZERO_CALIBRATE_REG_CNT;
  APP_LOG_DEBUG("按键任务0点校准指令消息.\r\n");
  }
  else
  {
  reg_addr=DEVICE_SPAN_CALIBRATE_REG_ADDR;  
  reg_cnt=DEVICE_SPAN_CALIBRATE_REG_CNT;
  APP_LOG_DEBUG("按键任务增益校准指令消息.校准重量：%d\r\n",calibrate_w);
  }
  APP_LOG_DEBUG("按键任务执行校准.\r\n");
  err_code=eMBMasterReqWriteMultipleHoldingRegister(calibrate_idx,reg_addr,reg_cnt,param,SWITCH_TASK_WAIT_TIMEOUT);
  if(err_code==MB_MRE_NO_ERR)
  {
  APP_LOG_DEBUG("按键任务执行校准成功.\r\n");
  osSignalSet(calibrate_memory_task_hdl,CALIBRATE_MEMORY_TASK_CALIBRATE_OK_SIGNAL);
  }
  else
  {
  APP_LOG_DEBUG("按键任务执行校准失败.\r\n");
  osSignalSet(calibrate_memory_task_hdl,CALIBRATE_MEMORY_TASK_CALIBRATE_ERR_SIGNAL);
  }
  osDelay(SWITCH_TASK_CALIBRATE_EXIT_WAIT_TIME);
  APP_LOG_DEBUG("切换为正常模式.\r\n");
  }
  else
  {
  APP_LOG_DEBUG("切换为校准模式.\r\n");
  osSignalSet(calibrate_memory_task_hdl,CALIBRATE_MEMORY_TASK_CALIBRATE_START_SIGNAL);/*通知校准显存任务校准开始*/
  }
  
  /*更新模式对应的显示缓存指针*/
  ptr_buff=switch_info.ptr_dis_buff[switch_info.idx];
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
 /*校准模式下校准按键长按功能等效正常模式长按*/
 APP_LOG_DEBUG("校准模式下校准按键长按.\r\n");  
 calibrate_sw_long_press_normal(); 
}
/*正常状态下去皮短按和长按功能函数*/
static void tare_sw_short_press_normal()
{
 /*去皮按键*/
 uint8_t w_idx;
 uint16_t param[2];
 eMBMasterReqErrCode err_code;
 
 w_idx=get_weight_memory_idx();
 param[0]=SCALE_AUTO_TARE_WEIGHT_VALUE>>16;
 param[1]=SCALE_AUTO_TARE_WEIGHT_VALUE&0xffff;
 APP_LOG_DEBUG("按键任务执行去皮.\r\n");
 err_code=eMBMasterReqWriteMultipleHoldingRegister(w_idx,DEVICE_TARE_WEIGHT_REG_ADDR,DEVICE_TARE_WEIGHT_REG_CNT,param,SWITCH_TASK_WAIT_TIMEOUT);
 if(err_code==MB_MRE_NO_ERR)
 {
 APP_LOG_DEBUG("按键任务执行去皮成功.\r\n");
 }
 else
 {
 APP_LOG_DEBUG("按键任务执行去皮失败.\r\n");
 }
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
 /*清零按键*/
 uint8_t w_idx;
 uint16_t param[2];
 eMBMasterReqErrCode err_code;
 
 w_idx=get_weight_memory_idx();
 param[0]=SCALE_CLEAR_ZERO_VALUE;
 APP_LOG_DEBUG("按键任务执行清零.\r\n");
 err_code=eMBMasterReqWriteMultipleHoldingRegister(w_idx,DEVICE_MANUALLY_CLEAR_REG_ADDR,DEVICE_MANUALLY_CLEAR_REG_CNT,param,SWITCH_TASK_WAIT_TIMEOUT);
 if(err_code==MB_MRE_NO_ERR)
 {
 APP_LOG_DEBUG("按键任务执行清零成功.\r\n");
 }
 else
 {
 APP_LOG_DEBUG("按键任务执行清零失败.\r\n");
 }
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
sw[WT_SWITCH_IDX].short_press_time=SWITCH_SHORT_PRESS_TIME;
sw[WT_SWITCH_IDX].long_press_time=SWITCH_LONG_PRESS_TIME;
sw[WT_SWITCH_IDX].short_press[0]=wt_sw_short_press_normal;
sw[WT_SWITCH_IDX].long_press[0]=wt_sw_long_press_normal;
sw[WT_SWITCH_IDX].short_press[1]=wt_sw_short_press_calibrate;
sw[WT_SWITCH_IDX].long_press[1]=wt_sw_long_press_calibrate;
/*重量层数选择按键*/
sw[W_SWITCH_IDX].short_press_time=SWITCH_SHORT_PRESS_TIME;
sw[W_SWITCH_IDX].long_press_time=SWITCH_LONG_PRESS_TIME;
sw[W_SWITCH_IDX].short_press[0]=w_sw_short_press_normal;
sw[W_SWITCH_IDX].long_press[0]=w_sw_long_press_normal;
sw[W_SWITCH_IDX].short_press[1]=w_sw_short_press_calibrate;
sw[W_SWITCH_IDX].long_press[1]=w_sw_long_press_calibrate;  
/*校准按键*/  
sw[CALIBRATE_SWITCH_IDX].short_press_time=SWITCH_SHORT_PRESS_TIME;
sw[CALIBRATE_SWITCH_IDX].long_press_time=SWITCH_LONG_PRESS_TIME;
sw[CALIBRATE_SWITCH_IDX].short_press[0]=calibrate_sw_short_press_normal; 
sw[CALIBRATE_SWITCH_IDX].long_press[0]=calibrate_sw_long_press_normal;
sw[CALIBRATE_SWITCH_IDX].short_press[1]=calibrate_sw_short_press_calibrate;
sw[CALIBRATE_SWITCH_IDX].long_press[1]=calibrate_sw_long_press_calibrate; 
/*去皮按键*/
sw[TARE_SWITCH_IDX].short_press_time=SWITCH_SHORT_PRESS_TIME;
sw[TARE_SWITCH_IDX].long_press_time=SWITCH_LONG_PRESS_TIME;
sw[TARE_SWITCH_IDX].short_press[0]=tare_sw_short_press_normal; 
sw[TARE_SWITCH_IDX].long_press[0]=tare_sw_long_press_normal;
sw[TARE_SWITCH_IDX].short_press[1]=tare_sw_short_press_calibrate;
sw[TARE_SWITCH_IDX].long_press[1]=tare_sw_long_press_calibrate;
/*清零按键*/
sw[ZERO_SWITCH_IDX].short_press_time=SWITCH_SHORT_PRESS_TIME;
sw[ZERO_SWITCH_IDX].long_press_time=SWITCH_LONG_PRESS_TIME;
sw[ZERO_SWITCH_IDX].short_press[0]=zero_sw_short_press_normal; 
sw[ZERO_SWITCH_IDX].long_press[0]=zero_sw_long_press_normal;
sw[ZERO_SWITCH_IDX].short_press[1]=zero_sw_short_press_calibrate;
sw[ZERO_SWITCH_IDX].long_press[1]=zero_sw_long_press_calibrate; 
APP_LOG_DEBUG("所有按键初始化完毕.\r\n");
}

/*按键扫描任务*/
void switch_task(void const * argument)
{
 APP_LOG_INFO("######按键状态任务开始.\r\n");
 /*初始化按键*/
 switch_init();
 /*默认显示初始化数据*/
 ptr_buff=init_dis_buff;
 /*从8-0依次显示，检测显示是否正常*/
 APP_LOG_DEBUG("初始化数码管显示...\r\n"); 
 for(uint8_t i=0x08;i>0;i--)
 {
  for(uint8_t j=0;j<DISPLAY_LED_POS_CNT;j++)
  {
  init_dis_buff[j].num=i;  
  init_dis_buff[j].dp=FORTUNA_TRUE;
  }
  osDelay(SWITCH_TASK_INIT_DISPLAY_HOLD_ON_TIME);
 }
 APP_LOG_DEBUG("初始化数码管显示完毕.\r\n");
 /*设为默认重量显示*/
 ptr_buff=switch_info.ptr_dis_buff[switch_info.idx];
 while(1)
 {   
 osDelay(SWITCH_TASK_INTERVAL);
 /*获取所有按键按键状态*/
 sw[WT_SWITCH_IDX].cur_state=BSP_get_wt_sw_state();
 sw[W_SWITCH_IDX].cur_state=BSP_get_w_sw_state();
 sw[CALIBRATE_SWITCH_IDX].cur_state=BSP_get_calibrate_sw_state();
 sw[TARE_SWITCH_IDX].cur_state=BSP_get_func1_sw_state();
 sw[ZERO_SWITCH_IDX].cur_state=BSP_get_func2_sw_state();
 
/*重量和温度选择没有实体按键 特殊处理 只在正常状态下有效*/
 if(sw[WT_SWITCH_IDX].cur_state==SW_STATE_PRESS)
 {
   if(ptr_buff==t_dis_buff)
     ptr_buff=switch_info.ptr_dis_buff[switch_info.idx];
 }
 else
 {
   if(ptr_buff!=t_dis_buff)
    ptr_buff=t_dis_buff;
 }
 /*除去温度和重量切换按键 处理其他按键的状态*/
 for(uint8_t i=W_SWITCH_IDX;i<SWITCH_CNT;i++)
 {
 if(sw[i].pre_state!=sw[i].cur_state)
 {
  if(sw[i].cur_state==SW_STATE_PRESS)/*因为有长短按压时间 所以按键定义为释放时有效*/ 
  {
   sw[i].pre_state=sw[i].cur_state;
   sw[i].hold_time=0;
  }
  else/*释放*/
  {
   if(sw[i].hold_time>=sw[i].long_press_time)/*长按释放*/
   {
    /*执行对应的长按功能*/
    if(sw[i].long_press[switch_info.idx])
    sw[i].long_press[switch_info.idx](); 
   }
   else if(sw[i].hold_time>=sw[i].short_press_time)/*短按释放*/
   {
    /*执行对应短按的功能*/
    if(sw[i].short_press[switch_info.idx])
    sw[i].short_press[switch_info.idx]();   
   }
   sw[i].hold_time=0;
  }
 }
 else if(sw[i].cur_state==SW_STATE_PRESS)/*只在按压下去时更新保持时间*/
 {
  sw[i].hold_time+=SWITCH_TASK_INTERVAL;/*更新保持时间 去抖动*/
 }
 }
 }
}