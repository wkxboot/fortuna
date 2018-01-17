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
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
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
 uint8_t scale,scale_end,scale_start;
 uint16_t param[2];
 uint16_t reg_addr,reg_cnt;
 eMBMasterReqErrCode err_code;
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
 scale=scale_msg.scale;
 if(scale==0)/*对所有的称*/
 {
  scale_start=1;
  scale_end=SCALES_CNT_MAX;
 }
 else
 {
  scale_start=scale; 
  scale_end=scale_start;  
 }
 switch(scale_msg.type)
 {
 case  SCALE_FUNC_TASK_CLEAR_ZERO_WEIGHT_MSG:
   param[0]=SCALE_CLEAR_ZERO_VALUE;
   param[1]=0;
   APP_LOG_DEBUG("收到清零消息.\r\n");
   for(scale=scale_start;scale<=scale_end;scale++)
   {
   APP_LOG_DEBUG("%2d#电子秤清零...\r\n",scale);
   err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MANUALLY_CLEAR_REG_ADDR,DEVICE_MANUALLY_CLEAR_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
   if(err_code!=MB_MRE_NO_ERR)
    break;
   APP_LOG_DEBUG("%2d#电子秤清零成功.\r\n",scale);
   }
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤清零成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送清零成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_ZERO_OK_SIGNAL);  
   }
   else
   {
   APP_LOG_DEBUG("%2d#电子秤清零失败.\r\n",scale);
   APP_LOG_DEBUG("向通信任务发送清零失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_ZERO_ERR_SIGNAL);  
   }
 case  SCALE_FUNC_TASK_CLEAR_TARE_WEIGHT_MSG:
    APP_LOG_DEBUG("收到去皮指令消息.\r\n");
    param[0]=SCALE_AUTO_TARE_WEIGHT_VALUE>>16;
    param[1]=SCALE_AUTO_TARE_WEIGHT_VALUE&0xffff;
    for(scale=scale_start;scale<=scale_end;scale++)
    {
    APP_LOG_DEBUG("%2d#电子秤去皮...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_TARE_WEIGHT_REG_ADDR,DEVICE_TARE_WEIGHT_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d#电子秤去皮成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤去皮成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送去皮成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("%2d#电子秤去皮失败.\r\n",scale);
   APP_LOG_DEBUG("向通信任务发送去皮失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_CALIBRATE_WEIGHT_MSG:
   param[0]=0;
   param[1]=scale_msg.param16;  
   if(param[1]==0)
   {
   reg_addr=DEVICE_ZERO_CALIBRATE_REG_ADDR;
   reg_cnt=DEVICE_ZERO_CALIBRATE_REG_CNT;
   APP_LOG_DEBUG("收到0点校准指令消息.\r\n");
   }
   else
   {
   reg_addr=DEVICE_SPAN_CALIBRATE_REG_ADDR;
   reg_cnt=DEVICE_SPAN_CALIBRATE_REG_CNT;
   APP_LOG_DEBUG("收到重量校准指令消息.\r\n");
   }
   for(scale=scale_start;scale<=scale_end;scale++)
   {
    APP_LOG_DEBUG("%2d#电子秤校准...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,reg_addr,reg_cnt,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d#电子秤校准成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤校准成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送校准成功信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("%2d#电子称校准失败.\r\n",scale);
   APP_LOG_DEBUG("向通信任务发送校准失败信号.\r\n");
   osSignalSet(host_comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG:  
   APP_LOG_DEBUG("收到获取净重消息.\r\n");
   for(scale=scale_start;scale<=scale_end;scale++)
   {
    APP_LOG_DEBUG("%2d#电子秤获取净重...\r\n",scale);
    err_code=eMBMasterReqReadHoldingRegister(scale,DEVICE_NET_WEIGHT_REG_ADDR,DEVICE_NET_WEIGHT_REG_CNT,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d#获取净重成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤获取净重成功.\r\n");
   APP_LOG_DEBUG("向净重轮询任务发送获取净重成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("%2d#获取净重失败.\r\n",scale);
   APP_LOG_DEBUG("向净重查询任务发送执行获取净重失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_OBTAIN_FIRMWARE_VERSION_MSG:
   APP_LOG_DEBUG("收到获取电子秤固件版本信息.\r\n");
   for(scale=scale_start;scale<=scale_end;scale++)
   {
    APP_LOG_DEBUG("%2d#电子秤获取固件版本...\r\n",scale);
    err_code=eMBMasterReqReadHoldingRegister(scale,DEVICE_FIRMWARE_VERTION_REG_ADDR,DEVICE_FIRMWARE_VERTION_REG_CNT,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d电子秤获取固件版本成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤获取固件版本成功.\r\n");
   APP_LOG_DEBUG("向重量任务发送获取固件版本成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_FIRMWARE_VERSION_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("%2d电子秤获取固件版本失败.\r\n",scale);
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_OBTAIN_FIRMWARE_VERSION_ERR_SIGNAL);
   }
   break; 
 case  SCALE_FUNC_TASK_SET_MAX_WEIGHT_MSG:
    APP_LOG_DEBUG("收到设置最大值指令消息.\r\n");
    param[0]=0;
    param[1]=scale_msg.param16;/*填入最大值*/
    for(scale=scale_start;scale<=scale_end;scale++)
    {
    APP_LOG_DEBUG("%2d#电子秤设置最大值...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MAX_WEIGHT_REG_ADDR,DEVICE_MAX_WEIGHT_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d#电子秤设置最大值成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤设置最大值成功.\r\n");
   APP_LOG_DEBUG("向重量轮询任务发送设置最大值成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_MAX_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("电子秤%d执行设置最大值失败.\r\n",scale);
   APP_LOG_DEBUG("向重量轮询任务发送设置最大值失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_MAX_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_SET_DIVISION_MSG:
    APP_LOG_DEBUG("收到设置分度值指令消息.\r\n");
    param[0]=scale_msg.param16;/*填入分度值*/
    for(scale=scale_start;scale<=scale_end;scale++)
    {
    APP_LOG_DEBUG("%2d#电子秤设置分度值...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_DIVISION_REG_ADDR,DEVICE_DIVISION_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d#电子秤设置分度值成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤设置分度值成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送设置分度值成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_DIVISION_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("%2d#电子秤设置分度值失败.\r\n",scale);
   APP_LOG_DEBUG("向通信任务发送设置分度值失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_SET_DIVISION_ERR_SIGNAL);
   }
   break;
 case  SCALE_FUNC_TASK_LOCK_MSG:
    param[0]=scale_msg.param16;/*填入锁指令参数*/   
    if(param[0]==SCALE_UNLOCK_VALUE)
    {
     APP_LOG_DEBUG("收到解锁指令消息.\r\n");
    }
    else
    {
    APP_LOG_DEBUG("收到上锁指令消息.\r\n"); 
    }
    for(scale=scale_start;scale<=scale_end;scale++)
    {
    APP_LOG_DEBUG("%2d#电子秤执行锁指令...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_LOCK_REG_ADDR,DEVICE_LOCK_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d#电子秤执行锁指令成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤执行锁指令成功.\r\n");
   APP_LOG_DEBUG("向通信任务发送执行锁指令成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_LOCK_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("%2d#电子秤执行锁指令失败.\r\n",scale);
   APP_LOG_DEBUG("向通信任务发送执行锁指令失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_LOCK_ERR_SIGNAL);
   }
   break;  
 case  SCALE_FUNC_TASK_RESET_MSG:
    APP_LOG_DEBUG("收到复位、出厂设置指令消息.\r\n");
    param[0]=SCALE_RESET_VALUE;/*填入复位参数*/
    for(scale=scale_start;scale<=scale_end;scale++)
    {
    APP_LOG_DEBUG("%2d#电子秤执行复位指令...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_RESET_REG_ADDR,DEVICE_RESET_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d#电子秤复位成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤复位成功.\r\n");
   APP_LOG_DEBUG("电子秤功能任务发送复位成功信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_RESET_OK_SIGNAL);
   }
   else
   {
   APP_LOG_DEBUG("%2d#电子秤复位失败.\r\n",scale);
   APP_LOG_DEBUG("电子秤功能任务发送执行复位指令失败信号.\r\n");
   osSignalSet(scale_poll_task_hdl,SCALE_POLL_TASK_RESET_ERR_SIGNAL);
   }
   break;  
 case  SCALE_FUNC_TASK_SET_ADDR_MSG:
    APP_LOG_DEBUG("收到设置地址指令消息.\r\n");
    if(scale==0)
    {
    APP_LOG_DEBUG("错误.不能同时设置多个地址.\r\n");
    break;
    }
    param[0]=scale_msg.param16;/*填入地址指令参数*/   
    APP_LOG_DEBUG("%2d#电子秤执行设置地址指令...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_ADDR_REG_ADDR,DEVICE_ADDR_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    /*执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
     APP_LOG_DEBUG("%2d#电子秤执行设置地址指令成功.\r\n",scale);
   /*
   APP_LOG_DEBUG("向xx发送执行设置地址指令成功信号.\r\n");
   osSignalSet(xx_task_hdl,xx_TASK_SET_ADDR_OK_SIGNAL);
   */
   }
   else
   {
   APP_LOG_DEBUG("电子秤%d执行设置地址指令失败.\r\n",scale); 
  /*
   APP_LOG_DEBUG("向xx任务发送执行设置地址指令失败信号.\r\n");
   osSignalSet(xx_task_hdl,xx_TASK_SET_ADDR_ERR_SIGNAL);
  */
   }
   break; 
 case  SCALE_FUNC_TASK_SET_BAUDRATE_MSG: 
    APP_LOG_DEBUG("收到设置波特率指令消息.\r\n");  
    param[0]=scale_msg.param16;/*填入波特率令参数*/   
    for(scale=scale_start;scale<=scale_end;scale++)
    {
    APP_LOG_DEBUG("%2d电子秤设置波特率...\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_BAUDRATE_REG_ADDR,DEVICE_BAUDRATE_REG_CNT,param,SCALE_FUNC_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("%2d电子秤设置波特率成功.\r\n",scale);
    }
   /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_DEBUG("所有电子秤执行设置波特率指令成功.\r\n");
   /*
   APP_LOG_DEBUG("向xx发送执行设置波特率指令成功信号.\r\n");
   osSignalSet(xx_task_hdl,xx_TASK_SET_ADDR_OK_SIGNAL);
   */
   }
   else
   {
   APP_LOG_DEBUG("%2d电子秤设置波特率失败.\r\n",scale);
  /*
   APP_LOG_DEBUG("向xx任务发送执行设置波特率指令失败信号.\r\n");
   osSignalSet(xx_task_hdl,xx_TASK_SET_ADDR_ERR_SIGNAL);
  */
   }
   break;   
   
 case  SCALE_FUNC_TASK_SET_FSM_FORMAT_MSG: 
 case  SCALE_FUNC_TASK_SET_PROTOCOL_FORMAT_MSG: 
 default:
   APP_LOG_DEBUG("没实现.\r\n");
  
 }  
}
}
