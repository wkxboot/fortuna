/*******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * modify by wkxboot 2017.7.11 
 * *****************************************************************************
 */
/* ----------------------- Modbus includes ----------------------------------*/

#include "port_m.h"
#include "mb_m.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f1xx.h"
#include "usart.h"
#define APP_LOG_MODULE_NAME   "[MB_M_SERIAL]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_INFO    
#include "app_log.h"
#include "app_error.h"
/* ----------------------- Defines ------------------------------------------*/

UART_HandleTypeDef * ptr_master_modbus_uart_handle;


#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Static variables ---------------------------------*/
/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortSerialInit(uint8_t port,uint32_t baudrate,uint8_t databits)
{
 if(port==1)
 {
  ptr_master_modbus_uart_handle=&huart1;
  ptr_master_modbus_uart_handle->Instance=USART1;
 }
 else if(port==2)
 {
  ptr_master_modbus_uart_handle=&huart2;
  ptr_master_modbus_uart_handle->Instance=USART2;  
 }
 else
 {
  ptr_master_modbus_uart_handle=&huart3;
  ptr_master_modbus_uart_handle->Instance=USART3;  
 }
  ptr_master_modbus_uart_handle->Init.BaudRate = baudrate;
  ptr_master_modbus_uart_handle->Init.WordLength =(databits==8?UART_WORDLENGTH_8B:UART_WORDLENGTH_9B);
  ptr_master_modbus_uart_handle->Init.StopBits = UART_STOPBITS_2;
  ptr_master_modbus_uart_handle->Init.Parity = UART_PARITY_NONE;
  ptr_master_modbus_uart_handle->Init.Mode = UART_MODE_TX_RX;
  ptr_master_modbus_uart_handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  ptr_master_modbus_uart_handle->Init.OverSampling = UART_OVERSAMPLING_16;
  if(HAL_UART_Init(ptr_master_modbus_uart_handle)!=HAL_OK)
  {
    APP_LOG_ERROR("MODBUS主机串口初始化失败.\r\n!");
    return FALSE;
  }
 APP_LOG_DEBUG("MODBUS主机串口初始化成功.\r\n!");
 return TRUE;
}

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
 
    if(xRxEnable)
    {
    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(ptr_master_modbus_uart_handle, UART_IT_RXNE);   
    APP_LOG_DEBUG("MODBUS主机使能接收.\r\n"); 
    }
    else
    {
    /* Enable the UART Transmit data register empty Interrupt */
    __HAL_UART_DISABLE_IT(ptr_master_modbus_uart_handle, UART_IT_RXNE);  
    APP_LOG_DEBUG("MODBUS主机禁止接收.\r\n");  
    }
   if(xTxEnable)
    {
   /* Enable the UART Transmit data register empty Interrupt */
    __HAL_UART_ENABLE_IT(ptr_master_modbus_uart_handle, UART_IT_TXE);   
     APP_LOG_DEBUG("MODBUS主机使能发送.\r\n"); 
    }
    else
    {
     /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_DISABLE_IT(ptr_master_modbus_uart_handle, UART_IT_TXE);   
    APP_LOG_DEBUG("MODBUS主机禁止发送.\r\n");
    }
}

void vMBMasterPortClose(void)
{
 APP_LOG_DEBUG("MODBUS主机关闭.\r\n"); 
}

BOOL xMBMasterPortSerialPutByte(CHAR ucByte)
{
  ptr_master_modbus_uart_handle->Instance->DR = ucByte;
  return TRUE;
}

BOOL xMBMasterPortSerialGetByte(CHAR * pucByte)
{
  *pucByte = (uint8_t)(ptr_master_modbus_uart_handle->Instance->DR & (uint8_t)0x00FF);
  return TRUE;
}

void MASTER_MODBUS_USARTIRQHandler( void )
{
uint32_t tmp_flag = 0, tmp_it_source = 0; 
  
  tmp_flag = __HAL_UART_GET_FLAG(ptr_master_modbus_uart_handle, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(ptr_master_modbus_uart_handle, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    pxMBMasterFrameCBByteReceived();
  }

  tmp_flag = __HAL_UART_GET_FLAG(ptr_master_modbus_uart_handle, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(ptr_master_modbus_uart_handle, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    pxMBMasterFrameCBTransmitterEmpty();
  }  
}
#endif
