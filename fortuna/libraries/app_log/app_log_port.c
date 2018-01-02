#include "cmsis_os.h"
#if APP_LOG_USE_UART > 0
#include "uart.h"
#endif
#include "app_log.h"

/*******************************************************************************
 1.app_log 移植文件;
 2.在IAR FreeRTOS MXcube 环境下使用;
 
 *******************************************************************************/


#if APP_LOG_USE_TIMESTAMP  >  0
uint32_t APP_TIMESTAMP(void)
{
  return osKernelSysTick();
}
#endif

#if APP_LOG_USE_SEGGER_RTT > 0
void SEGGER_RTT_CRITICAL_REGION_ENTER()      
{                               
  if(__get_IPSR()==0)                  
  {                                  
   taskENTER_CRITICAL();                                              
 }  
}

void SEGGER_RTT_CRITICAL_REGION_EXIT()   
{                                  
  if ( __get_IPSR()==0)                 
  {                                  
   taskEXIT_CRITICAL();              
  }                                  
}
#endif


#if APP_LOG_USE_UART > 0
void APP_LOG_UART_INIT(void)
{
}
int fputc(int ch, FILE *f)
{
 HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,0xff);
 return ch;
}
#endif
