#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "comm_protocol.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#include "host_comm_task.h"
#include "scale_comm_task.h"
#include "scale_func_task.h"
#include "scale_poll_task.h"
#include "lock_task.h"
#define APP_LOG_MODULE_NAME   "[host_comm]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


osThreadId host_comm_task_hdl;
extern EventGroupHandle_t task_sync_evt_group_hdl;

/*与主机通信任务*/
void host_comm_task(void const * argument)
{
 osEvent signals;
 uint8_t *ptr_buff,recv_len,send_len;
 comm_status_t status;
 APP_LOG_INFO("######与主机通信任务开始.\r\n");
 status=comm_init(COMM_ADDR,COMM_PORT,COMM_BAUDRATE,COMM_DATABITS);
 if(status!=COMM_OK)
 {
  APP_LOG_ERROR("与主机通信任务初始化失败.\r\n");
  APP_ERROR_HANDLER(APP_ERROR_INIT);
 }
 APP_LOG_INFO("与主机通信任务等待同步...\r\n");
 xEventGroupSync(task_sync_evt_group_hdl,HOST_COMM_TASK_SYNC_EVT,SCALE_POLL_TASK_SYNC_EVT  |\
                                                                  SCALE_FUNC_TASK_SYNC_EVT |\
                                                                  SCALE_COMM_TASK_SYNC_EVT |\
                                                                  HOST_COMM_TASK_SYNC_EVT,  \
                                                                  osWaitForever);

 APP_LOG_INFO("与主机通信任务同步完成.\r\n");
 while(1)
 {
 signals=osSignalWait(HOST_COMM_TASK_ALL_SIGNALS,osWaitForever);
 if(signals.status!=osEventSignal)
 continue;
 /*处理通信缓存溢出信号*/
 if(signals.value.signals & HOST_COMM_TASK_BUFF_OVERFLOW_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到串口缓存溢出信号.\r\n"); 
 }
 /*处理接收数据帧信号*/
 if(signals.value.signals & HOST_COMM_TASK_RECV_FSM_SIGNAL)
 {
 APP_LOG_DEBUG("通信任务收到串口数据帧信号.开始接收.\r\n");
 status=comm_receive_fsm(&ptr_buff,&recv_len);
 if(status==COMM_OK)
 {
  APP_LOG_DEBUG("通信任务接收数据成功.\r\n"); 
  APP_LOG_DEBUG("向通信任务发送解析协议信号.\r\n"); 
  osSignalSet(host_comm_task_hdl,HOST_COMM_TASK_PARSE_PROTOCOL_SIGNAL);
 }
 }
 /*处理解析数据帧信号*/
 if(signals.value.signals & HOST_COMM_TASK_PARSE_PROTOCOL_SIGNAL)
 {
 status=comm_protocol_parse(ptr_buff,recv_len,&send_len);
 if(status==COMM_OK)
 {
 APP_LOG_DEBUG("协议解析成功.处理成功.\r\n");
 APP_LOG_DEBUG("向通信任务发送串口数据帧发送信号.\r\n");
 osSignalSet(host_comm_task_hdl,HOST_COMM_TASK_SEND_FSM_SIGNAL);
 }
 }
 /*处理发送数据帧信号*/
 if(signals.value.signals & HOST_COMM_TASK_SEND_FSM_SIGNAL)
 {
  APP_LOG_DEBUG("通信任务收到发送串口数据帧信号.\r\n"); 
  /*启动串口发送中断*/
  APP_LOG_DEBUG("启动发送串口数据.\r\n");
  status=comm_send_fsm(ptr_buff,send_len);
  if(status==COMM_OK)
  {
  APP_LOG_DEBUG("串口数据发送中...\r\n");
  }
 }
 /*处理发送数据帧完毕信号*/
 if(signals.value.signals & HOST_COMM_TASK_SEND_FSM_OVER_SIGNAL)
 {
  APP_LOG_DEBUG("通信任务收到发送串口数据帧完毕信号.\r\n"); 
 }
}
}
