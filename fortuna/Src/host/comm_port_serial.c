#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "usart.h"
#include "comm_protocol.h"
#include "host_comm_task.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#include "usart.h"
#define APP_LOG_MODULE_NAME   "[port_serial]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"


UART_HandleTypeDef *ptr_comm_serial_handle;

comm_status_t xcomm_port_serial_init(uint8_t port,uint32_t baudrate,uint8_t databits)
{
 if(port==1)
 {
  ptr_comm_serial_handle=&huart1;
  ptr_comm_serial_handle->Instance=USART1;
 }
 else if(port==2)
 {
  ptr_comm_serial_handle=&huart2;
  ptr_comm_serial_handle->Instance=USART2;  
 }
 else
 {
  ptr_comm_serial_handle=&huart3;
  ptr_comm_serial_handle->Instance=USART3;  
 }
  ptr_comm_serial_handle->Init.BaudRate = baudrate;
  ptr_comm_serial_handle->Init.WordLength =(databits==8?UART_WORDLENGTH_8B:UART_WORDLENGTH_9B);
  ptr_comm_serial_handle->Init.StopBits = UART_STOPBITS_1;
  ptr_comm_serial_handle->Init.Parity = UART_PARITY_NONE;
  ptr_comm_serial_handle->Init.Mode = UART_MODE_TX_RX;
  ptr_comm_serial_handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  ptr_comm_serial_handle->Init.OverSampling = UART_OVERSAMPLING_16;
  if(HAL_UART_Init(ptr_comm_serial_handle)!=HAL_OK)
  {
   return COMM_ERR;
  }
 return COMM_OK;
}

void xcomm_port_serial_enable(fortuna_bool_t rx_bool,fortuna_bool_t tx_bool)
{
 if(rx_bool)
  {
 /*使能接收中断*/
  __HAL_UART_ENABLE_IT(ptr_comm_serial_handle,UART_IT_RXNE);
  APP_LOG_DEBUG("通信使能接收中断.\r\n"); 
  }
 else
 {
  __HAL_UART_DISABLE_IT(ptr_comm_serial_handle,UART_IT_RXNE); 
  APP_LOG_DEBUG("通信禁止接收中断.\r\n"); 
 }
 if(tx_bool)
 {
  /*使能发送中断*/
  __HAL_UART_ENABLE_IT(ptr_comm_serial_handle,UART_IT_TXE);   
  APP_LOG_DEBUG("通信使能发送中断.\r\n"); 
 }
 else
 {
 /*禁止发送中断*/
 __HAL_UART_DISABLE_IT(ptr_comm_serial_handle, UART_IT_TXE);   
 APP_LOG_DEBUG("通信禁止发送中断.\r\n"); 
 }
}

void xcomm_port_serial_send_byte(uint8_t send_byte)
{
 ptr_comm_serial_handle->Instance->DR = send_byte;
}

void xcomm_port_serial_get_byte(uint8_t *ptr_byte)
{
 *ptr_byte = (uint8_t)(ptr_comm_serial_handle->Instance->DR & (uint8_t)0x00FF);
}

void xcomm_port_serial_isr(void)
{
  uint32_t tmp_flag = 0, tmp_it_source = 0; 
  
  tmp_flag = __HAL_UART_GET_FLAG(ptr_comm_serial_handle, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(ptr_comm_serial_handle, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
   comm_byte_receive();
  }

  tmp_flag = __HAL_UART_GET_FLAG(ptr_comm_serial_handle, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(ptr_comm_serial_handle, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
   comm_byte_send();
  }  
}