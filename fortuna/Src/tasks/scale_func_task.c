#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "comm_protocol.h"
#include "host_comm_task.h"
#include "scale_func_task.h"
#include "scale_poll_task.h"
#include "scale_comm_task.h"
#include "scales.h"
#include "mb_m.h"
#define APP_LOG_MODULE_NAME   "[scales_func]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"

osThreadId scale_func_task_hdl;
osMessageQId scale_func_msg_q_id;
extern EventGroupHandle_t task_sync_evt_group_hdl;
/*
 * 电子秤功能操作任务
 */
void scale_func_task(void const * argument)
{
 osEvent msg;
 scale_msg_t scale_msg;
 fortuna_bool_t ret;
 APP_LOG_INFO("######电子秤功能任务开始.\r\n");
 /*创建自己的消息队列*/
 osMessageQDef(scale_func_task_msg,6,uint32_t);
 scale_func_msg_q_id=osMessageCreate(osMessageQ(scale_func_task_msg),scale_func_task_hdl);
 APP_ASSERT(scale_func_msg_q_id);

 APP_LOG_INFO("功能任务等待同步...\r\n");
 xEventGroupSync(task_sync_evt_group_hdl,SCALE_FUNC_TASK_SYNC_EVT,SCALE_POLL_TASK_SYNC_EVT |\
                                                                  SCALE_FUNC_TASK_SYNC_EVT |\
                                                                  SCALE_COMM_TASK_SYNC_EVT |\
                                                                  HOST_COMM_TASK_SYNC_EVT,  \
                                                                  osWaitForever);

 APP_LOG_INFO("功能任务同步完成.\r\n");
 while(1)
 {
 msg=osMessageGet(scale_func_msg_q_id,osWaitForever);
 if(msg.status!=osEventMessage)
 continue;
 scale_msg=*(scale_msg_t*)&msg.value.v;
 switch(scale_msg.type)
 {
 case  SCALE_FUNC_TASK_CLEAR_ZERO_WEIGHT_MSG:
   APP_LOG_DEBUG("收到清零消息.\r\n");
   ret=scale_clear_zero(scale_msg.scale,scale_msg.param16);
   if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤清零成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送清零成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_ZERO_OK_SIGNAL);  
   }
   else
   {
   APP_LOG_DEBUG("电子秤清零失败.\r\n");
   APP_LOG_DEBUG("向通信任务发送清零失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_ZERO_ERR_SIGNAL);  
   }
 case  SCALE_FUNC_TASK_CLEAR_TARE_WEIGHT_MSG:
    APP_LOG_DEBUG("收到去皮指令消息.\r\n");
   if(scale_msg.param16==0)/*设置皮重为0*/
   ret=scale_remove_tare(scale_msg.scale,0); 
   else/*自动取当前毛重值为皮重*/
   ret=scale_remove_tare(scale_msg.scale,SCALE_AUTO_TARE_WEIGHT_VALUE); 
    /*全部执行成功*/
   if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤去皮成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送去皮成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子秤去皮失败.\r\n");
   APP_LOG_DEBUG("向通信任务发送去皮失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_CALIBRATE_CODE_MSG:
   APP_LOG_DEBUG("收到标定内码消息.\r\n");
   ret=scale_calibrate_code(scale_msg.scale,scale_msg.param16);
    /*全部执行成功*/
  if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤标定内码成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送标定内码成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_CODE_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子称标定内码失败.\r\n");
   APP_LOG_DEBUG("向通信任务发送标定内码失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_CODE_ERR_SIGNAL);
   }
   break;
  
  case  SCALE_FUNC_TASK_CALIBRATE_MEASUREMENT_MSG:
   APP_LOG_DEBUG("收到标定测量值消息.\r\n");
   ret=scale_calibrate_measurement(scale_msg.scale,scale_msg.param16);
    /*全部执行成功*/
  if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤标定测量值成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送标定测量值成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子称标定测量值失败.\r\n");
   APP_LOG_DEBUG("向通信任务发送标定测量值失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_ERR_SIGNAL);
   }
   break;
   case  SCALE_FUNC_TASK_CALIBRATE_WEIGHT_MSG:
   APP_LOG_DEBUG("收到标定重量消息.\r\n");
   ret=scale_calibrate_weight(scale_msg.scale,scale_msg.param16);
    /*全部执行成功*/
  if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤标定重量成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送标定重量成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子称标定重量失败.\r\n");
   APP_LOG_DEBUG("向通信任务发送标定重量失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_WEIGHT_ERR_SIGNAL);
   }
   break;
  
  
 case  SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG:  
   APP_LOG_DEBUG("收到获取净重消息.\r\n");
  /*电子秤获取净重操作*/
   ret= scale_obtain_net_weight(scale_msg.scale,scale_msg.param16);
    /*全部执行成功*/
   if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤获取净重成功.\r\n");
   APP_LOG_DEBUG("向净重轮询任务发送获取净重成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("获取净重失败.\r\n");
   APP_LOG_DEBUG("向净重查询任务发送执行获取净重失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_OBTAIN_FIRMWARE_VERSION_MSG:
   APP_LOG_DEBUG("收到获取电子秤固件版本信息.\r\n");
   ret= scale_obtain_firmware_version(scale_msg.scale,scale_msg.param16);
   /*全部执行成功*/
   if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤获取固件版本成功.\r\n");
   APP_LOG_DEBUG("向重量任务发送获取固件版本成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_FIRMWARE_VERSION_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子秤获取固件版本失败.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_FIRMWARE_VERSION_ERR_SIGNAL);
   }
   break; 
 case  SCALE_FUNC_TASK_SET_MAX_WEIGHT_MSG:
    APP_LOG_DEBUG("收到设置最大值指令消息.\r\n");
    ret= scale_set_max_weight(scale_msg.scale,scale_msg.param16);  
    /*全部执行成功*/
   if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤设置最大值成功.\r\n");
   APP_LOG_DEBUG("向重量轮询任务发送设置最大值成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_MAX_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子秤设置最大值失败.\r\n");
   APP_LOG_DEBUG("向重量轮询任务发送设置最大值失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_MAX_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_SET_DIVISION_MSG:
    APP_LOG_DEBUG("收到设置分度值指令消息.\r\n");
    ret= scale_set_division(scale_msg.scale,scale_msg.param16);  
    /*全部执行成功*/
   if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤设置分度值成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送设置分度值成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_DIVISION_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子秤设置分度值失败.\r\n");
   APP_LOG_DEBUG("向通信任务发送设置分度值失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_DIVISION_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_LOCK_MSG:  
   APP_LOG_DEBUG("收到锁操作消息.\r\n");
   ret= scale_lock_operation(scale_msg.scale,scale_msg.param16);  
    /*全部执行成功*/
   if(ret==FORTUNA_TRUE)
   {
   APP_LOG_DEBUG("电子秤执行锁指令成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送执行锁指令成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_LOCK_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子秤执行锁指令失败.\r\n");
   APP_LOG_DEBUG("向通信任务发送执行锁指令失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_LOCK_ERR_SIGNAL);
   }
   break;  
 case  SCALE_FUNC_TASK_RESET_MSG:  
 case  SCALE_FUNC_TASK_SET_ADDR_MSG:
 case  SCALE_FUNC_TASK_SET_BAUDRATE_MSG:   
 case  SCALE_FUNC_TASK_SET_FSM_FORMAT_MSG: 
 case  SCALE_FUNC_TASK_SET_PROTOCOL_FORMAT_MSG: 
 default:
 APP_LOG_DEBUG("没实现.\r\n");
  
 }  
}
}
