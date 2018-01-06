#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#include "communication.h"
#include "communication_task.h"
#include "electronic_scales_task.h"
#include "lock_task.h"
#define APP_LOG_MODULE_NAME   "[comm_task]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

/*与上位机的通信任务*/
void communication_task(void const * argument)
{
 osEvent signals;
 uint8_t *ptr_buff,recv_len,send_len;
 comm_status_t status;
 APP_LOG_INFO("######上位机通信任务开始.\r\n");
 status=comm_init(COMM_ADDR,COMM_PORT,COMM_BAUDRATE,COMM_DATABITS);
 if(status!=COMM_OK)
 {
 APP_LOG_ERROR("通信任务初始化失败.\r\n");
 }
 while(1)
 {
 signals=osSignalWait(COMM_TASK_ALL_SIGNALS,osWaitForever);
 if(signals.status!=osEventSignal)
 continue;
 /*处理通信缓存溢出信号*/
 if(signals.value.signals & COMM_TASK_BUFF_OVERFLOW_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到串口缓存溢出信号.\r\n"); 
 }
 /*处理接收数据帧信号*/
 if(signals.value.signals & COMM_TASK_RECV_FSM_SIGNAL)
 {
 APP_LOG_ERROR("通信任务收到串口数据帧信号.开始接收.\r\n");
 status=comm_receive_fsm(&ptr_buff,&recv_len);
 if(status==COMM_OK)
 {
  APP_LOG_ERROR("通信任务接收数据成功.\r\n"); 
  APP_LOG_ERROR("向通信任务发送解析协议信号.\r\n"); 
  osSignalSet(comm_task_hdl,COMM_TASK_PARSE_PROTOCOL_SIGNAL);
 }
 }
 /*处理解析数据帧信号*/
 if(signals.value.signals & COMM_TASK_PARSE_PROTOCOL_SIGNAL)
 {
 status=comm_protocol_parse(ptr_buff,recv_len,&send_len);
 if(status==COMM_OK)
 {
 APP_LOG_INFO("协议解析完成.\r\n");
 APP_LOG_INFO("向通信任务发送发送数据帧信号.\r\n");
 osSignalSet(comm_task_hdl,COMM_TASK_SEND_FSM_SIGNAL);
 }
 }
 /*处理发送数据帧信号*/
 if(signals.value.signals & COMM_TASK_SEND_FSM_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到发送串口数据帧信号.\r\n"); 
  send_len+=COMM_ADDR_SIZE;
  
  xcomm_port_serial_enable(FORTUNA_FALSE,FORTUNA_TRUE);
  comm_send_fsm(ptr_buff,send_len);
 }
 /*处理发送数据帧完毕信号*/
 if(signals.value.signals & COMM_TASK_SEND_FSM_OVER_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到发送串口数据帧完毕信号.\r\n"); 
  xcomm_port_serial_enable(FORTUNA_TRUE,FORTUNA_FALSE);
 }
}
}
