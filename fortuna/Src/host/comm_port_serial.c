#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "usart.h"
#include "host_protocol.h"
#include "host_comm_task.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#define APP_LOG_MODULE_NAME   "[port_serial]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"

static volatile uint8_t serial_buff[SERIAL_BUFF_MAX];
static volatile uint8_t *ptr_send_buff;/*发送数据时缓存位置指针*/
static volatile uint8_t serial_recv_cnt,serial_send_cnt;/*发送和接收的数据大小*/

UART_HandleTypeDef *ptr_comm_serial_handle=&huart1;

comm_status_t xcomm_port_serial_init(uint8_t port,uint32_t baudrate,uint8_t databits)
{
 MX_USART1_UART_Init();
 return COMM_OK;
}

void xcomm_port_serial_enable(fortuna_bool_t rx_bool,fortuna_bool_t tx_bool)
{
 if(rx_bool)
  {
 /*使能接收中断*/
  __HAL_UART_ENABLE_IT(ptr_comm_serial_handle,UART_IT_RXNE);
  APP_LOG_WARNING("通信使能接收中断.\r\n"); 
  }
 else
 {
  __HAL_UART_DISABLE_IT(ptr_comm_serial_handle,UART_IT_RXNE); 
  APP_LOG_WARNING("通信禁止接收中断.\r\n"); 
 }
 if(tx_bool)
 {
  /*使能发送中断*/
  __HAL_UART_ENABLE_IT(ptr_comm_serial_handle,UART_IT_TXE);   
  APP_LOG_WARNING("通信使能发送中断.\r\n"); 
 }
 else
 {
 /*禁止发送中断*/
 __HAL_UART_DISABLE_IT(ptr_comm_serial_handle, UART_IT_TXE);   
 APP_LOG_WARNING("通信禁止发送中断.\r\n"); 
 }
}

static void xcomm_port_serial_send_byte(uint8_t send_byte)
{
 ptr_comm_serial_handle->Instance->DR = send_byte;
}

static void xcomm_port_serial_get_byte(uint8_t *ptr_byte)
{
 *ptr_byte = (uint8_t)(ptr_comm_serial_handle->Instance->DR & (uint8_t)0x00FF);
}

static void xcomm_port_serial_isr_receive(void)
{
 uint8_t recv_byte;
 xcomm_port_serial_get_byte(&recv_byte);
 if(serial_recv_cnt>=SERIAL_BUFF_MAX)
 {
 APP_LOG_ERROR("串口缓存溢出.本次数据无效.\r\n");
 serial_recv_cnt=0;
 }
 serial_buff[serial_recv_cnt++]=recv_byte;
 /*重新开始定时器*/
 xcomm_port_serial_timer_start();
}

static void xcomm_port_serial_isr_send(void)
{
if(serial_send_cnt!=0)
{
xcomm_port_serial_send_byte(*ptr_send_buff); 
ptr_send_buff++;/*下一个字节*/
serial_send_cnt--;/*更新待发送数量*/
}
else
{
 osSignalSet(host_comm_task_hdl,HOST_COMM_TASK_SEND_FSM_OVER_SIGNAL);  
}
}

/*获取接收一帧串口数据的地址和长度*/
void xcomm_port_serial_recv(uint8_t **ptr_buff,uint8_t *ptr_recv_len)
{
 *ptr_recv_len=serial_recv_cnt;
 *ptr_buff=(uint8_t*)serial_buff; 
}
/*发送一帧串口数据*/
void xcomm_port_serial_send(uint8_t *ptr_buff,uint8_t send_len)
{
serial_send_cnt=send_len;
ptr_send_buff=ptr_buff;
xcomm_port_serial_isr_send();
}

void xcomm_port_serial_isr(void)
{
  uint32_t tmp_flag = 0, tmp_it_source = 0; 
  
  tmp_flag = __HAL_UART_GET_FLAG(ptr_comm_serial_handle, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(ptr_comm_serial_handle, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
   xcomm_port_serial_isr_receive();
  }

  tmp_flag = __HAL_UART_GET_FLAG(ptr_comm_serial_handle, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(ptr_comm_serial_handle, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
  xcomm_port_serial_isr_send();
  }  
}