#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "electronic_scales_task.h"
#include "scales_reg.h"
#include "mb_m.h"
#define APP_LOG_MODULE_NAME   "[elec_scales]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"

/*
 * 电子秤轮询任务
 */
void elec_scales_task(void const * argument)
{
 APP_LOG_DEBUG("######电子秤操作任务开始.");
 osEvent msg;
 scale_msg_t scale_msg;
 eMBMasterReqErrCode err_code;
 while(1)
 {
 msg=osMessageGet(elec_scales_task_hdl,osWaitForever);
 if(msg.status!=osEventMessage)
 continue;
 scale_msg=*(scale_msg_t*)&msg.value.v;
 switch(scale_msg.type)
 {
 case  COMM_CLEAR_SCALE_TARE_WEIGHT_MSG:
  
eMBMasterReqWriteMultipleHoldingRegister( UCHAR ucSndAddr,
		USHORT usRegAddr, USHORT usNRegs, USHORT * pusDataBuffer, LONG lTimeOut ) 
 break;
 case  COMM_CALIBRATE_SCALE_WEIGHT_MSG:
 case  COMM_UNLOCK_LOCK_MSG:
 case  COMM_LOCK_LOCK_MSG:
   break;
 default:
   
 }
  
  
  
  
}
}
