#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "host_comm_task.h"
#include "scale_comm_task.h"
#include "scale_func_task.h"
#include "scale_poll_task.h"
#include "scales.h"
#define APP_LOG_MODULE_NAME   "[poll]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"

osThreadId scale_poll_task_hdl;
osMessageQId scale_poll_msg_q_id;

extern EventGroupHandle_t task_sync_evt_group_hdl;
/*
 * 电子秤轮询任务
 */
void scale_poll_task(void const * argument)
{
 app_bool_t ret;
 APP_LOG_INFO("@电子秤轮询任务开始.\r\n");
 /*创建自己的消息队列*/
 osMessageQDef(scale_poll_task_msg,6,uint32_t);
 scale_poll_msg_q_id=osMessageCreate(osMessageQ(scale_poll_task_msg),scale_poll_task_hdl);
 APP_ASSERT(scale_poll_msg_q_id);
 
 APP_LOG_INFO("轮询任务等待同步...\r\n");
 xEventGroupSync(task_sync_evt_group_hdl,SCALE_POLL_TASK_SYNC_EVT,SCALE_POLL_TASK_SYNC_EVT |\
                                                                  SCALE_FUNC_TASK_SYNC_EVT |\
                                                                  SCALE_COMM_TASK_SYNC_EVT |\
                                                                  HOST_COMM_TASK_SYNC_EVT,  \
                                                                  osWaitForever);

 APP_LOG_INFO("轮询任务同步完成.\r\n");
/*净重量轮询*/
 APP_LOG_INFO("电子秤循环获取净重值...\r\n");
 /*临时调试暂停*/
 /*
 while(1)
 {
  osDelay(100);
 }
 */
 while(1)
 {
  ret= scale_obtain_net_weight(1,0);
   /*全部执行成功*/
  if(ret==APP_TRUE)
  {
  APP_LOG_DEBUG("电子秤获取净重成功.\r\n");
  }
  else
  {
  APP_LOG_ERROR("获取净重失败.\r\n");
  } 
  osDelay(SCALE_POLL_TASK_INTERVAL);
 }
}
   
   
   
   
   
   
