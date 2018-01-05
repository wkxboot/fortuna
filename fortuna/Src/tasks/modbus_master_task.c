#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "modbus_master_task.h"
#include "mb_m.h"
#define APP_LOG_MODULE_NAME   "[modbus_master]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


/*
 * 电子秤轮询任务
 */
void modbus_master_task(void const * argument)
{
  APP_LOG_DEBUG("######MODBUS主机任务开始.\r\n");
  /*这个地方初始化参数无效，因为在usart.c已经初始化。为了移植.*/
  eMBMasterInit(MB_MASTER_RTU,0,115200,8);
  /* Enable the Modbus Protocol Stack. */
  eMBMasterEnable();
  for(;;)
 {
  /* Call the main polling loop of the Modbus protocol stack. */  
  eMBMasterPoll();
 } 
}



