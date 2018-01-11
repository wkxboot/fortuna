#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
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
 osEvent signal;
 scale_msg_t scale_msg;
 uint16_t time;
 APP_LOG_INFO("######电子秤轮询任务开始.\r\n");
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
 while(1)
 {
  osDelay(100);
 }
 
 while(1)
 {
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG;
 scale_msg.scale=0;/*全部的电子秤*/
 /*向电子秤功能任务发送净重值消息*/
 while(time<SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
 APP_LOG_DEBUG("向电子秤功能任务发送获取净重值消息.\r\n");
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&scale_msg,0);
 signal=osSignalWait(SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL,SCALE_POLL_TASK_WAIT_TIMEOUT);
 if(signal.status==osEventSignal && signal.value.signals & SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL)/*超时或者其他错误*/
 break;
 time+=SCALE_POLL_TASK_WAIT_TIMEOUT;
 }
 if(time >= SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
 APP_LOG_ERROR("电子秤获取净重超时.再次尝试.\r\n");
 }
 else
 {
 APP_LOG_INFO("电子秤获取净重值成功.\r\n");
 }
 }
}