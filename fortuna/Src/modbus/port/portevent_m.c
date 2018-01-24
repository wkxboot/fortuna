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
#include "semphr.h"
#include "event_groups.h"
#define APP_LOG_MODULE_NAME   "[MB_M_EVT]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_ERROR    
#include "app_log.h"

#define  EVENT_WAIT_FOREVER      0xFFFFFFFF

extern  volatile uint32_t mb_response_time;
 // Declare a variable to hold the created event group.
EventGroupHandle_t xMasterOsEvent;

typedef struct
{
  SemaphoreHandle_t xSemaphore;
}xSemaphoreInternalHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC xSemaphoreInternalHandle arxSemaphorehdls[1];

/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Variables ----------------------------------------*/
/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBMasterPortEventInit( void )
{
  BOOL bOk=TRUE;
  // Attempt to create the event group.
  xMasterOsEvent = xEventGroupCreate();

// Was the event group created successfully?
    if( xMasterOsEvent == NULL )
    {
	bOk=FALSE;	// The event group was not created because there was insufficient
	APP_LOG_ERROR("MODBUS主机事件组初始化失败.\r\n");// FreeRTOS heap available.
    }
    {
    APP_LOG_DEBUG("MODBUS主机事件组初始化成功.\r\n")
    }
    return bOk;
}

BOOL
xMBMasterPortEventPost( eMBMasterEventType eEvent )
{
   // EventBits_t uxBits;
     APP_LOG_DEBUG("MODBUS发送事件：%d.\r\n",eEvent);
     xEventGroupSetBits(xMasterOsEvent,	// The event group being updated.
			        eEvent );// The bits being set.
    return TRUE;
}

BOOL
xMBMasterPortEventGet( eMBMasterEventType * peEvent )
{
    EventBits_t uxBits;
    /* waiting forever OS event */

    APP_LOG_DEBUG("MODBUS等待事件.\r\n");
    uxBits = xEventGroupWaitBits(xMasterOsEvent,// The event group being tested.
			            EV_MASTER_READY | EV_MASTER_FRAME_RECEIVED | EV_MASTER_EXECUTE |EV_MASTER_FRAME_SENT | EV_MASTER_ERROR_PROCESS,// The bits within the event group to wait for.
				        pdTRUE,// should be cleared before returning.
				        pdFALSE,// Don't wait for both bits, either bit will do.
				        EVENT_WAIT_FOREVER);// Wait forever either bit to be set.
  
    //*peEvent = (eMBMasterEventType)uxBits;
    switch (uxBits)
    {
    case EV_MASTER_READY:
        *peEvent = EV_MASTER_READY;
        break;
    case EV_MASTER_FRAME_RECEIVED:
        *peEvent = EV_MASTER_FRAME_RECEIVED;
        break;
    case EV_MASTER_EXECUTE:
        *peEvent = EV_MASTER_EXECUTE;
        break;
    case EV_MASTER_FRAME_SENT:
        *peEvent = EV_MASTER_FRAME_SENT;
        break;
    case EV_MASTER_ERROR_PROCESS:
        *peEvent = EV_MASTER_ERROR_PROCESS;
        break;
    }
    return TRUE;
      
}

BOOL  vMBMasterOsResInit( void )
{
  BOOL  bOk=FALSE;
      // Semaphore cannot be used before a call to vSemaphoreCreateBinary ().
    // This is a macro so pass the variable in directly.
    arxSemaphorehdls[0].xSemaphore = xSemaphoreCreateBinary();

    if( arxSemaphorehdls[0].xSemaphore != NULL )
    {
        // The semaphore was created successfully.
        // The semaphore can now be used.
     bOk=TRUE;   
     APP_LOG_DEBUG("MODBUS主机信号量初始化成功.\r\n" );
     vMBMasterRunResRelease();//release semaphore    
    }
    else
    {
      APP_LOG_ERROR("MODBUS主机信号量初始化失败.\r\n" );
    }
    return bOk;
}
/**
 * This function is take Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be just return TRUE.
 *
 * @param lTimeOut the waiting time.
 *
 * @return resource taked result
 */
BOOL xMBMasterRunResTake( LONG lTimeOut )
{
    BOOL bOk=FALSE;
    TickType_t ticks = lTimeOut / portTICK_PERIOD_MS;
    /*If waiting time is -1 .It will wait forever */
  if( xSemaphoreTake( arxSemaphorehdls[0].xSemaphore, ticks ) == pdTRUE)
  {
    bOk=TRUE;
    APP_LOG_DEBUG("MODBUS主机获取信号量成功.\r\n" );
    mb_response_time=ticks;/*根据实际情况赋值回应参数*/
  }
  else
  {
   APP_LOG_ERROR("MODBUS主机获取信号量失败.\r\n" );
  }
  
    return bOk ;
}
/**
 * This function is release Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be empty.
 *
 */
BOOL vMBMasterRunResRelease( void )
{
   BOOL bOk=FALSE;
    /* release resource */
   if( xSemaphoreGive( arxSemaphorehdls[0].xSemaphore ) == pdTRUE )
   {
    bOk=TRUE;                 // We would expect this call to fail because we cannot give
    APP_LOG_DEBUG("MODBUS主机释放信号量成功.\r\n" );// a semaphore without first "taking" it!
   }
   else
   {
   APP_LOG_ERROR("MODBUS主机释放信号量失败.\r\n" );// a semaphore without first "taking" it!
   }

   return bOk;
}
/**
 * This is modbus master respond timeout error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBRespondTimeout(UCHAR ucDestAddress, const UCHAR* pucPDUData,
        USHORT ucPDULength) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_ERROR_RESPOND_TIMEOUT);

    /* You can add your code under here. */

}

/**
 * This is modbus master receive data error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBReceiveData(UCHAR ucDestAddress, const UCHAR* pucPDUData,
        USHORT ucPDULength) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost( EV_MASTER_ERROR_RECEIVE_DATA);

    /* You can add your code under here. */

}

/**
 * This is modbus master execute function error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBExecuteFunction(UCHAR ucDestAddress, const UCHAR* pucPDUData,
        USHORT ucPDULength) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_ERROR_EXECUTE_FUNCTION);

    /* You can add your code under here. */

}

/**
 * This is modbus master request process success callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 */
void vMBMasterCBRequestSuccess( void ) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_PROCESS_SUCCESS);

    /* You can add your code under here. */

}

/**
 * This function is wait for modbus master request finish and return result.
 * Waiting result include request process success, request respond timeout,
 * receive data error and execute function error.You can use the above callback function.
 * @note If you are use OS, you can use OS's event mechanism. Otherwise you have to run
 * much user custom delay for waiting.
 *
 * @return request error code
 */
eMBMasterReqErrCode eMBMasterWaitRequestFinish( void )
{ 
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;
    /* waiting for OS event */
    EventBits_t uxBits;
    APP_LOG_DEBUG("MODBUS等待请求结束.\r\n");
    uxBits = xEventGroupWaitBits(   xMasterOsEvent,	// The event group being tested.
			            EV_MASTER_PROCESS_SUCCESS | EV_MASTER_ERROR_RESPOND_TIMEOUT | EV_MASTER_ERROR_RECEIVE_DATA |EV_MASTER_ERROR_EXECUTE_FUNCTION ,// The bits within the event group to wait for.
				    pdTRUE,		// should be cleared before returning.
				    pdFALSE,		// Don't wait for both bits, either bit will do.
				    EVENT_WAIT_FOREVER );	// Wait forever either bit to be set.
 
    switch (uxBits)
    {
    case EV_MASTER_PROCESS_SUCCESS:
        break;
    case EV_MASTER_ERROR_RESPOND_TIMEOUT:
    {
        eErrStatus = MB_MRE_TIMEDOUT;
        break;
    }
    case EV_MASTER_ERROR_RECEIVE_DATA:
    {
        eErrStatus = MB_MRE_REV_DATA;
        break;
    }
    case EV_MASTER_ERROR_EXECUTE_FUNCTION:
    {
        eErrStatus = MB_MRE_EXE_FUN;
        break;
    }
    }
    return eErrStatus;
}

