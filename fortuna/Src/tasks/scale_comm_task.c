#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "host_comm_task.h"
#include "scale_comm_task.h"
#include "scale_func_task.h"
#include "scale_poll_task.h"
#include "mb_m.h"
#define APP_LOG_MODULE_NAME   "[modbus_master]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId scale_comm_task_hdl;
extern EventGroupHandle_t task_sync_evt_group_hdl;

/*
 * 电子秤通信任务
 */
void scale_comm_task(void const * argument)
{
  APP_LOG_INFO("######电子秤MODBUS主机通信任务开始.\r\n");
  /*初始化参数.*/
  eMBMasterInit(MB_MASTER_RTU,5,9600,8);
  /* Enable the Modbus Protocol Stack. */
  eMBMasterEnable();
  /*等待MODBUS初始化完毕*/
  osDelay(10);
  APP_LOG_INFO("MODBUS主机任务等待同步...\r\n");
  xEventGroupSync(task_sync_evt_group_hdl,SCALE_COMM_TASK_SYNC_EVT,SCALE_POLL_TASK_SYNC_EVT |\
                                                                   SCALE_FUNC_TASK_SYNC_EVT |\
                                                                   SCALE_COMM_TASK_SYNC_EVT |\
                                                                   HOST_COMM_TASK_SYNC_EVT,  \
                                                                   osWaitForever);

  APP_LOG_INFO("MODBUS主机任务同步完成.\r\n");
  for(;;)
 {
  /* Call the main polling loop of the Modbus protocol stack. */  
  eMBMasterPoll();
 } 
}



