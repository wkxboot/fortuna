#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "host_comm_task.h"
#include "scales.h"
#include "scale_func_task.h"
#include "scale_poll_task.h"
#define APP_LOG_MODULE_NAME   "[poll]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId scale_poll_task_hdl;
osMessageQId scale_poll_msg_q_id;

/*
 * 重量任务
 */
void scale_poll_task(void const * argument)
{
 osEvent signal;
 scale_msg_t scale_msg;
 uint16_t time;
 /*创建自己的消息队列*/
 osMessageQDef(scale_poll_task_msg,6,uint32_t);
 scale_poll_msg_q_id=osMessageCreate(osMessageQ(scale_poll_task_msg),scale_func_task_hdl);
 APP_ASSERT(scale_poll_msg_q_id);
 APP_LOG_DEBUG("######电子秤轮询任务开始.\r\n");

 osDelay(2000);/*临时等待其他任务*/
/*净重量轮询*/
 APP_LOG_INFO("电子秤循环获取净重值...");
 while(1)
 {
 time=0;
 scale_msg.type=SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG;
 scale_msg.scale=0;/*全部的电子秤*/
 /*向电子秤功能任务发送净重值消息*/
 while(time<SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_TIMEOUT)
 {
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