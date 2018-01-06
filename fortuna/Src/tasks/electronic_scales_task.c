#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "electronic_scales_task.h"
#include "communication_task.h"
#include "weight_task.h"
#include "communication.h"
#include "scales.h"
#include "mb_m.h"
#define APP_LOG_MODULE_NAME   "[elec_scales]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"

/*
 * 电子秤操作任务
 */
void elec_scales_task(void const * argument)
{
 APP_LOG_DEBUG("######电子秤操作任务开始.");
 osEvent msg;
 scale_msg_t scale_msg;
 uint8_t scale,scale_cnt,scale_start;
 uint16_t param[2];
 uint16_t reg_addr,reg_cnt;
 eMBMasterReqErrCode err_code;
 while(1)
 {
 msg=osMessageGet(elec_scales_task_hdl,osWaitForever);
 if(msg.status!=osEventMessage)
 continue;
 scale_msg=*(scale_msg_t*)&msg.value.v;
 scale=scale_msg.scale;
 if(scale==0)/*对所有的称去皮*/
 {
  scale_start=1;
  scale_cnt=SCALES_CNT_MAX;
 }
 else
 {
  scale_start=scale; 
  scale_cnt=scale_start;  
 }
 switch(scale_msg.type)
 {
 case  ELEC_SCALE_TASK_SET_TARE_WEIGHT_MSG:
     
 case  ELEC_SCALE_TASK_CLEAR_TARE_WEIGHT_MSG:
    APP_LOG_DEBUG("电子秤任务收到去皮指令消息.\r\n");
    param[0]=ELEC_SCALE_TASK_AUTO_TARE_WEIGHT_VALUE>>16;
    param[1]=ELEC_SCALE_TASK_AUTO_TARE_WEIGHT_VALUE&0xffff;
    for(scale=scale_start;scale<=scale_cnt;scale++)
    {
    APP_LOG_DEBUG("电子秤%d执行去皮.\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_TARE_WEIGHT_REG_ADDR,DEVICE_TARE_WEIGHT_REG_CNT,param,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行去皮成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行去皮成功.\r\n");
   APP_LOG_INFO("向通信任务发送执行去皮成功信号.\r\n");
   osSignalSet(comm_task_hdl,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d执行去皮失败.\r\n",scale);
   APP_LOG_INFO("向通信任务发送执行去皮失败信号.\r\n");
   osSignalSet(comm_task_hdl,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  ELEC_SCALE_TASK_CALIBRATE_WEIGHT_MSG:
   param[0]=0;
   if(scale_msg.param16==0)
   {
   param[1]=0;
   reg_addr=DEVICE_ZERO_CALIBRATE_REG_ADDR;
   reg_cnt=DEVICE_ZERO_CALIBRATE_REG_CNT;
   APP_LOG_DEBUG("电子秤任务收到0点校准指令消息.\r\n");
   }
   else
   {
   param[1]=0;
   reg_addr=DEVICE_SPAN_CALIBRATE_REG_ADDR;
   reg_cnt=DEVICE_SPAN_CALIBRATE_REG_CNT;
   APP_LOG_DEBUG("电子秤任务收到重量校准指令消息.\r\n");
   }
   for(scale=scale_start;scale<=scale_cnt;scale++)
   {
    APP_LOG_DEBUG("电子秤%d执行校准.\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,reg_addr,reg_cnt,param,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行校准成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行校准成功.\r\n");
   APP_LOG_INFO("向通信任务发送执行校准成功信号.\r\n");
   osSignalSet(comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d校准请求执行失败.\r\n",scale); 
   APP_LOG_INFO("向通信任务发送执行校准失败信号.\r\n");
   osSignalSet(comm_task_hdl,COMM_TASK_CALIBRATE_SCALE_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  ELEC_SCALE_TASK_OBTAIN_WEIGHT_MSG:  
   for(scale=scale_start;scale<=scale_cnt;scale++)
   {
    APP_LOG_DEBUG("电子秤%d执行获取净重.\r\n",scale);
    err_code=eMBMasterReqReadHoldingRegister(scale,DEVICE_NET_WEIGHT_REG_ADDR,DEVICE_NET_WEIGHT_REG_CNT,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行获取净重成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行获取净重成功.\r\n");
   APP_LOG_INFO("净重查询任务发送执行获取净重成功信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_OBTAIN_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d执行获取净重失败.\r\n",scale); 
   APP_LOG_INFO("向净重查询任务发送执行获取净重失败信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_OBTAIN_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  ELEC_SCALE_TASK_OBTAIN_FIRMWARE_VERSION_MSG:
   for(scale=scale_start;scale<=scale_cnt;scale++)
   {
    APP_LOG_DEBUG("电子秤%d执行获取固件版本.\r\n",scale);
    err_code=eMBMasterReqReadHoldingRegister(scale,DEVICE_FIRMWARE_VERTION_REG_ADDR,DEVICE_FIRMWARE_VERTION_REG_CNT,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行获取固件版本成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行获取固件版本成功.\r\n");
   APP_LOG_INFO("向重量任务发送获取固件版本成功信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_OBTAIN_FIRMWARE_VERSION_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d执行获取固件版本失败.\r\n",scale);
   osSignalSet(weight_task_hdl,WEIGHT_TASK_OBTAIN_FIRMWARE_VERSION_ERR_SIGNAL);
   }
   break; 
 case  ELEC_SCALE_TASK_SET_MAX_WEIGHT_MSG:
    APP_LOG_DEBUG("电子秤任务收到设置最大值指令消息.\r\n");
    param[0]=0;
    param[1]=scale_msg.param16;/*填入最大值*/
    for(scale=scale_start;scale<=scale_cnt;scale++)
    {
    APP_LOG_DEBUG("电子秤%d执行设置最大值.\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MAX_WEIGHT_REG_ADDR,DEVICE_MAX_WEIGHT_REG_CNT,param,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行设置最大值成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行设置最大值成功.\r\n");
   APP_LOG_INFO("向重量任务发送执行去皮成功信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_SET_MAX_WEIGHT_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d执行设置最大值失败.\r\n",scale);
   APP_LOG_INFO("向重量任务发送执行设置最大值失败信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_SET_MAX_WEIGHT_ERR_SIGNAL);
   }
   break;
 case  ELEC_SCALE_TASK_SET_DIVISION_MSG:
    APP_LOG_DEBUG("电子秤任务收到设置分度值指令消息.\r\n");
    param[0]=0;
    param[1]=scale_msg.param16;/*填入最大值*/
    for(scale=scale_start;scale<=scale_cnt;scale++)
    {
    APP_LOG_DEBUG("电子秤%d执行设置分度值.\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_DIVISION_REG_ADDR,DEVICE_DIVISION_REG_CNT,param,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行设置分度值成功.\r\n",scale);
    }
    /*全部执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行设置分度值成功.\r\n");
   APP_LOG_INFO("向通信任务发送执行设置分度值成功信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_SET_DIVISION_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d执行设置分度值失败.\r\n",scale);
   APP_LOG_INFO("向通信任务发送执行设置分度值失败信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_SET_DIVISION_ERR_SIGNAL);
   }
   break;
 case  ELEC_SCALE_TASK_LOCK_MSG:
    APP_LOG_DEBUG("电子秤任务收到锁指令消息.\r\n");
    param[0]=0;
    param[1]=scale_msg.param16;/*填入锁指令参数*/
    for(scale=scale_start;scale<=scale_cnt;scale++)
    {
    APP_LOG_DEBUG("电子秤%d执行锁指令.\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_LOCK_REG_ADDR,DEVICE_LOCK_REG_CNT,param,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行锁指令成功.\r\n",scale);
    }
    /*全部去皮指令执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行锁指令成功.\r\n");
   APP_LOG_INFO("向通信任务发送执行锁指令成功信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_SET_DIVISION_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d执行锁指令失败.\r\n",scale);
   APP_LOG_INFO("向通信任务发送执行锁指令失败信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_SET_DIVISION_ERR_SIGNAL);
   }
   break;  
 case  ELEC_SCALE_TASK_RESET_MSG:
    APP_LOG_DEBUG("电子秤任务收到复位指令消息.\r\n");
    param[0]=ELEC_SCALE_TASK_RESET_VALUE;/*填入复位参数*/
    for(scale=scale_start;scale<=scale_cnt;scale++)
    {
    APP_LOG_DEBUG("电子秤%d执行复位指令.\r\n",scale);
    err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_LOCK_REG_ADDR,DEVICE_LOCK_REG_CNT,param,ELEC_SCALE_TASK_WAIT_TIMEOUT);
    if(err_code!=MB_MRE_NO_ERR)
    break;
    APP_LOG_DEBUG("电子秤%d执行复位指令成功.\r\n",scale);
    }
    /*全部去皮指令执行成功*/
   if(err_code==MB_MRE_NO_ERR)
   {
   APP_LOG_INFO("所有电子秤执行复位指令成功.\r\n");
   APP_LOG_INFO("重量查询任务发送执行复位指令成功信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_RESET_OK_SIGNAL);
   }
   else
   {
   APP_LOG_ERROR("电子秤%d执行复位指令失败.\r\n",scale);
   APP_LOG_INFO("重量查询任务发送执行复位指令失败信号.\r\n");
   osSignalSet(weight_task_hdl,WEIGHT_TASK_RESET_ERR_SIGNAL);
   }
   break;  
 case  ELEC_SCALE_TASK_SET_ADDR_MSG:
 case  ELEC_SCALE_TASK_SET_BAUDRATE_MSG: 
 case  ELEC_SCALE_TASK_SET_FSM_FORMAT_MSG: 
 case  ELEC_SCALE_TASK_SET_PROTOCOL_FORMAT_MSG: 
 default:
   APP_LOG_DEBUG("没实现.\r\n");
  
 }  
}
}
