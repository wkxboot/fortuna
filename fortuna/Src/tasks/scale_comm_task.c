#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "scales.h"
#include "host_comm_task.h"
#include "scale_comm_task.h"
#include "scale_func_task.h"
#include "scale_poll_task.h"
#include "modbus_poll.h"
#include "ABDK_AHG081_ZK.h"
#define APP_LOG_MODULE_NAME   "[modbus_poll]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"

osThreadId scale_comm_task_hdl;
extern EventGroupHandle_t task_sync_evt_group_hdl;

/*
 * 电子秤通信任务
 */
void scale_comm_task(void const * argument)
{
  APP_LOG_INFO("@电子秤MODBUS主机通信任务开始.\r\n");
  /*初始化参数.*/
  modbus_poll_init();
  /*RS485通信半双工模式 发送前RS485为发送状态*/
  modbus_poll_pre_transmission(BSP_RS485_TX_ENABLE); 
  /*RS485通信半双工模式 接收前RS485为发送状态*/
  modbus_poll_post_transmission(BSP_RS485_RX_ENABLE);
  
  /*电子秤初始化*/
  scale_init();
  APP_LOG_INFO("MODBUS POLL任务等待同步...\r\n");
  xEventGroupSync(task_sync_evt_group_hdl,SCALE_COMM_TASK_SYNC_EVT,SCALE_POLL_TASK_SYNC_EVT |\
                                                                   SCALE_FUNC_TASK_SYNC_EVT |\
                                                                   SCALE_COMM_TASK_SYNC_EVT |\
                                                                   HOST_COMM_TASK_SYNC_EVT,  \
                                                                   osWaitForever);

  APP_LOG_INFO("MODBUS主机任务同步完成.\r\n");
  for(;;)
  {
  osDelay(1);
  } 
}



