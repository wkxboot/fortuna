#ifndef __MB_PORT_M_H__
#define __MB_PORT_M_H__

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    EV_MASTER_READY                    = 1<<0,  /*!< Startup finished. */
    EV_MASTER_FRAME_RECEIVED           = 1<<1,  /*!< Frame received. */
    EV_MASTER_EXECUTE                  = 1<<2,  /*!< Execute function. */
    EV_MASTER_FRAME_SENT               = 1<<3,  /*!< Frame sent. */
    EV_MASTER_ERROR_PROCESS            = 1<<4,  /*!< Frame error process. */
    EV_MASTER_PROCESS_SUCCESS          = 1<<5,  /*!< Request process success. */
    EV_MASTER_ERROR_RESPOND_TIMEOUT    = 1<<6,  /*!< Request respond timeout. */
    EV_MASTER_ERROR_RECEIVE_DATA       = 1<<7,  /*!< Request receive data error. */
    EV_MASTER_ERROR_EXECUTE_FUNCTION   = 1<<8,  /*!< Request execute function error. */
} eMBMasterEventType;

typedef enum
{
    EV_ERROR_RESPOND_TIMEOUT,         /*!< Slave respond timeout. */
    EV_ERROR_RECEIVE_DATA,            /*!< Receive frame data erroe. */
    EV_ERROR_EXECUTE_FUNCTION,        /*!< Execute function error. */
} eMBMasterErrorEventType;

/*! \ingroup modbus
 * \brief Parity used for characters in serial mode.
 *
 * The parity which should be applied to the characters sent over the serial
 * link. Please note that this values are actually passed to the porting
 * layer and therefore not all parity modes might be available.
 */
typedef enum
{
    MB_MASTER_PAR_NONE,                /*!< No parity. */
    MB_MASTER_PAR_ODD,                 /*!< Odd parity. */
    MB_MASTER_PAR_EVEN                 /*!< Even parity. */
} eMB_MASTER_Parity;




/* ----------------------- Supporting functions -----------------------------*/

BOOL            xMBMasterPortEventInit( void );

BOOL            xMBMasterPortEventPost( eMBMasterEventType eEvent );

BOOL            xMBMasterPortEventGet(  /*@out@ */ eMBMasterEventType * eEvent );
BOOL            vMBMasterOsResInit( void );

BOOL            xMBMasterRunResTake( LONG time );

BOOL            vMBMasterRunResRelease( void );

/* ----------------------- Serial port functions ----------------------------*/


BOOL            xMBMasterPortSerialInit(uint8_t port,uint32_t baudrate,uint8_t databits);

void            vMBMasterPortClose( void );

void            xMBMasterPortSerialClose( void );

void            vMBMasterPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable );

BOOL     xMBMasterPortSerialGetByte( CHAR * pucByte );

BOOL     xMBMasterPortSerialPutByte( CHAR ucByte );

/* ----------------------- Timers functions ---------------------------------*/

BOOL            xMBMasterPortTimersInit();

void            xMBMasterPortTimersClose( void );

void     vMBMasterPortTimersT35Enable( void );

void     vMBMasterPortTimersConvertDelayEnable( void );

void     vMBMasterPortTimersRespondTimeoutEnable( void );

void     vMBMasterPortTimersDisable( void );

/* ----------------- Callback for the master error process ------------------*/
void            vMBMasterErrorCBRespondTimeout( UCHAR ucDestAddress, const UCHAR* pucPDUData,
                                                USHORT ucPDULength );

void            vMBMasterErrorCBReceiveData( UCHAR ucDestAddress, const UCHAR* pucPDUData,
                                             USHORT ucPDULength );

void            vMBMasterErrorCBExecuteFunction( UCHAR ucDestAddress, const UCHAR* pucPDUData,
                                                 USHORT ucPDULength );

void            vMBMasterCBRequestSuccess( void );

/* ----------------------- Callback for the protocol stack ------------------*/

/*!
 * \brief Callback function for the porting layer when a new byte is
 *   available.
 *
 * Depending upon the mode this callback function is used by the RTU or
 * ASCII transmission layers. In any case a call to xMBPortSerialGetByte()
 * must immediately return a new character.
 *
 * \return <code>TRUE</code> if a event was posted to the queue because
 *   a new byte was received. The port implementation should wake up the
 *   tasks which are currently blocked on the eventqueue.
 */

extern          BOOL( *pxMBMasterFrameCBByteReceived ) ( void );

extern          BOOL( *pxMBMasterFrameCBTransmitterEmpty ) ( void );

extern          BOOL( *pxMBMasterPortCBTimerExpired ) ( void );

/* ----------------------- TCP port functions -------------------------------*/
BOOL            xMB_MASTER_TCPPortInit( USHORT usTCPPort );

void            vMB_MASTER_TCPPortClose( void );

void            vMB_MASTER_TCPPortDisable( void );

BOOL            xMB_MASTER_TCPPortGetRequest( UCHAR **ppucMB_MASTER_TCPFrame, USHORT * usTCPLength );

BOOL            xMB_MASTER_TCPPortSendResponse( const UCHAR *pucMB_MASTER_TCPFrame, USHORT usTCPLength );

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
