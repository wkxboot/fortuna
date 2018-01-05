#ifndef __MB_M_H__
#define __MB_M_H__

#include "port_m.h"
#include "mbport_m.h"
#include "mbconfig_m.h"
#include "mbproto_m.h"
#include "mbfunc_m.h"
   
#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

/*! \defgroup modbus Modbus
 * \code #include "mb_m.h" \endcode
 *
 * This module defines the interface for the application. It contains
 * the basic functions and types required to use the Modbus Master protocol stack.
 * A typical application will want to call eMBMasterInit() first. If the device
 * is ready to answer network requests it must then call eMBEnable() to activate
 * the protocol stack. In the main loop the function eMBMasterPoll() must be called
 * periodically. The time interval between pooling depends on the configured
 * Modbus timeout. If an RTOS is available a separate task should be created
 * and the task should always call the function eMBMasterPoll().
 *
 * \code
 * // Initialize protocol stack in RTU mode for a Master
 * eMBMasterInit( MB_RTU, 38400, MB_PAR_EVEN );
 * // Enable the Modbus Protocol Stack.
 * eMBMasterEnable(  );
 * for( ;; )
 * {
 *     // Call the main polling loop of the Modbus Master protocol stack.
 *     eMBMasterPoll(  );
 *     ...
 * }
 * \endcode
 */

/* ----------------------- Defines ------------------------------------------*/

/*! \ingroup modbus
 * \brief Use the default Modbus Master TCP port (502)
 */
#define MB_MASTER_TCP_PORT_USE_DEFAULT 0
/*! \ingroup modbus
 * \brief Modbus serial transmission modes (RTU/ASCII).
 *
 * Modbus serial supports two transmission modes. Either ASCII or RTU. RTU
 * is faster but has more hardware requirements and requires a network with
 * a low jitter. ASCII is slower and more reliable on slower links (E.g. modems)
 */
    typedef enum
{
    MB_MASTER_RTU,                     /*!< RTU transmission mode. */
    MB_MASTER_ASCII,                   /*!< ASCII transmission mode. */
    MB_MASTER_TCP                      /*!< TCP mode. */
} eMB_MASTER_Mode;

/*! \ingroup modbus
 * \brief If register should be written or read.
 *
 * This value is passed to the callback functions which support either
 * reading or writing register values. Writing means that the application
 * registers should be updated and reading means that the modbus protocol
 * stack needs to know the current register values.
 *
 * \see eMB_MASTER_RegHoldingCB( ), eMB_MASTER_RegCoilsCB( ), eMB_MASTER_RegDiscreteCB( ) and 
 *   eMB_MASTER_RegInputCB( ).
 */
typedef enum
{
    MB_MASTER_REG_READ,                /*!< Read register values and pass to protocol stack. */
    MB_MASTER_REG_WRITE                /*!< Update register values. */
} eMB_MASTER_RegisterMode;

/*! \ingroup modbus
 * \brief Errorcodes used by all function in the protocol stack.
 */
typedef enum
{
    MB_MASTER_ENOERR,                  /*!< no error. */
    MB_MASTER_ENOREG,                  /*!< illegal register address. */
    MB_MASTER_EINVAL,                  /*!< illegal argument. */
    MB_MASTER_EPORTERR,                /*!< porting layer error. */
    MB_MASTER_ENORES,                  /*!< insufficient resources. */
    MB_MASTER_EIO,                     /*!< I/O error. */
    MB_MASTER_EILLSTATE,               /*!< protocol stack in illegal state. */
    MB_MASTER_ETIMEDOUT                /*!< timeout error occurred. */
} eMB_MASTER_ErrorCode;

/* ----------------------- Type definitions ---------------------------------*/
/*! \ingroup modbus
 * \brief Errorcodes used by all function in the Master request.
 */
typedef enum
{
    MB_MRE_NO_ERR,                  /*!< no error. */
    MB_MRE_NO_REG,                  /*!< illegal register address. */
    MB_MRE_ILL_ARG,                 /*!< illegal argument. */
    MB_MRE_REV_DATA,                /*!< receive data error. */
    MB_MRE_TIMEDOUT,                /*!< timeout error occurred. */
    MB_MRE_MASTER_BUSY,             /*!< master is busy now. */
    MB_MRE_EXE_FUN                  /*!< execute function error. */
} eMBMasterReqErrCode;
/*! \ingroup modbus
 *  \brief TimerMode is Master 3 kind of Timer modes.
 */
typedef enum
{
	MB_TMODE_T35,                   /*!< Master receive frame T3.5 timeout. */
	MB_TMODE_RESPOND_TIMEOUT,       /*!< Master wait respond for slave. */
	MB_TMODE_CONVERT_DELAY          /*!< Master sent broadcast ,then delay sometime.*/
}eMBMasterTimerMode;

/* ----------------------- Function prototypes ------------------------------*/
/*! \ingroup modbus
 * \brief Initialize the Modbus Master protocol stack.
 *
 * This functions initializes the ASCII or RTU module and calls the
 * init functions of the porting layer to prepare the hardware. Please
 * note that the receiver is still disabled and no Modbus frames are
 * processed until eMBMasterEnable( ) has been called.
 *
 * \param eMode If ASCII or RTU mode should be used.
 * \param ucPort The port to use. E.g. 1 for COM1 on windows. This value
 *   is platform dependent and some ports simply choose to ignore it.
 * \param ulBaudRate The baudrate. E.g. 19200. Supported baudrates depend
 *   on the porting layer.
 * \param eParity Parity used for serial transmission.
 *
 * \return If no error occurs the function returns eMBErrorCode::MB_ENOERR.
 *   The protocol is then in the disabled state and ready for activation
 *   by calling eMBMasterEnable( ). Otherwise one of the following error codes
 *   is returned:
 *    - eMBErrorCode::MB_EPORTERR IF the porting layer returned an error.
 */
eMB_MASTER_ErrorCode    eMBMasterInit(eMB_MASTER_Mode eMode,uint8_t port,uint32_t baudrate,uint8_t databits);

/*! \ingroup modbus
 * \brief Initialize the Modbus Master protocol stack for Modbus TCP.
 *
 * This function initializes the Modbus TCP Module. Please note that
 * frame processing is still disabled until eMB_MASTER_Enable( ) is called.
 *
 * \param usTCPPort The TCP port to listen on.
 * \return If the protocol stack has been initialized correctly the function
 *   returns eMB_MASTER_ErrorCode::MB_MASTER__ENOERR. Otherwise one of the following error
 *   codes is returned:
 *    - eMB_MASTER_ErrorCode::MB_MASTER__EINVAL If the slave address was not valid. Valid
 *        slave addresses are in the range 1 - 247.
 *    - eMB_MASTER_ErrorCode::MB_MASTER__EPORTERR IF the porting layer returned an error.
 */
eMB_MASTER_ErrorCode    eMBMasterTCPInit( USHORT usTCPPort );

/*! \ingroup modbus
 * \brief Release resources used by the protocol stack.
 *
 * This function disables the Modbus Master protocol stack and release all
 * hardware resources. It must only be called when the protocol stack 
 * is disabled. 
 *
 * \note Note all ports implement this function. A port which wants to 
 *   get an callback must define the macro MB_PORT_HAS_CLOSE to 1.
 *
 * \return If the resources where released it return eMBErrorCode::MB_ENOERR.
 *   If the protocol stack is not in the disabled state it returns
 *   eMBErrorCode::MB_EILLSTATE.
 */
eMB_MASTER_ErrorCode    eMBMasterClose( void );

/*! \ingroup modbus
 * \brief Enable the Modbus Master protocol stack.
 *
 * This function enables processing of Modbus Master frames. Enabling the protocol
 * stack is only possible if it is in the disabled state.
 *
 * \return If the protocol stack is now in the state enabled it returns 
 *   eMB_MASTER_ErrorCode::MB_ENOERR. If it was not in the disabled state it 
 *   return eMBErrorCode::MB_EILLSTATE.
 */
eMB_MASTER_ErrorCode    eMBMasterEnable( void );

/*! \ingroup modbus
 * \brief Disable the Modbus Master protocol stack.
 *
 * This function disables processing of Modbus frames.
 *
 * \return If the protocol stack has been disabled it returns 
 *  eMB_MASTER_ErrorCode::MB_MASTER__ENOERR. If it was not in the enabled state it returns
 *  eMB_MASTER_ErrorCode::MB_MASTER__EILLSTATE.
 */
eMB_MASTER_ErrorCode    eMBMasterDisable( void );

/*! \ingroup modbus
 * \brief The main pooling loop of the Modbus Master protocol stack.
 *
 * This function must be called periodically. The timer interval required
 * is given by the application dependent Modbus slave timeout. Internally the
 * function calls xMBMasterPortEventGet() and waits for an event from the receiver or
 * transmitter state machines. 
 *
 * \return If the protocol stack is not in the enabled state the function
 *   returns eMB_MASTER_ErrorCode::MB_MASTER__EILLSTATE. Otherwise it returns 
 *   eMB_MASTER_ErrorCode::MB_MASTER__ENOERR.
 */
eMB_MASTER_ErrorCode    eMBMasterPoll( void );

/*! \ingroup modbus
 * \brief Registers a callback handler for a given function code.
 *
 * This function registers a new callback handler for a given function code.
 * The callback handler supplied is responsible for interpreting the Modbus PDU and
 * the creation of an appropriate response. In case of an error it should return
 * one of the possible Modbus exceptions which results in a Modbus exception frame
 * sent by the protocol stack.
 *
 * \param ucFunctionCode The Modbus function code for which this handler should
 *   be registers. Valid function codes are in the range 1 to 127.
 * \param pxHandler The function handler which should be called in case
 *   such a frame is received. If \c NULL a previously registered function handler
 *   for this function code is removed.
 *
 * \return eMB_MASTER_ErrorCode::MB_MASTER__ENOERR if the handler has been installed. If no
 *   more resources are available it returns eMB_MASTER_ErrorCode::MB_MASTER__ENORES. In this
 *   case the values in MB_MASTER_config.h should be adjusted. If the argument was not
 *   valid it returns eMB_MASTER_ErrorCode::MB_MASTER__EINVAL.
 */
eMB_MASTER_ErrorCode    eMBMasterRegisterCB( UCHAR ucFunctionCode,
                               pxMB_MASTER_FunctionHandler pxHandler );

/* ----------------------- Callback -----------------------------------------*/

/*! \defgroup modbus_master registers Modbus Registers
 * \code #include "mb_m.h" \endcode
 * The protocol stack does not internally allocate any memory for the
 * registers. This makes the protocol stack very small and also usable on
 * low end targets. In addition the values don't have to be in the memory
 * and could for example be stored in a flash.<br>
 * Whenever the protocol stack requires a value it calls one of the callback
 * function with the register address and the number of registers to read
 * as an argument. The application should then read the actual register values
 * (for example the ADC voltage) and should store the result in the supplied
 * buffer.<br>
 * If the protocol stack wants to update a register value because a write
 * register function was received a buffer with the new register values is
 * passed to the callback function. The function should then use these values
 * to update the application register values.
 */

/*! \ingroup modbus_registers
 * \brief Callback function used if the value of a <em>Input Register</em>
 *   is required by the protocol stack. The starting register address is given
 *   by \c usAddress and the last register is given by <tt>usAddress +
 *   usNRegs - 1</tt>.
 *
 * \param pucRegBuffer A buffer where the callback function should write
 *   the current value of the modbus registers to.
 * \param usAddress The starting address of the register. Input registers
 *   are in the range 1 - 65535.
 * \param usNRegs Number of registers the callback function must supply.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_MASTER__ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMB_MASTER_ErrorCode::MB_MASTER__ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMB_MASTER_ErrorCode eMBMasterRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress,
		USHORT usNRegs );

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Holding Register</em> value is
 *   read or written by the protocol stack. The starting register address
 *   is given by \c usAddress and the last register is given by
 *   <tt>usAddress + usNRegs - 1</tt>.
 *
 * \param pucRegBuffer If the application registers values should be updated the
 *   buffer points to the new registers values. If the protocol stack needs
 *   to now the current values the callback function should write them into
 *   this buffer.
 * \param usAddress The starting address of the register.
 * \param usNRegs Number of registers to read or write.
 * \param eMode If eMBRegisterMode::MB_REG_WRITE the application register
 *   values should be updated from the values in the buffer. For example
 *   this would be the case when the Modbus master has issued an
 *   <b>WRITE SINGLE REGISTER</b> command.
 *   If the value eMBRegisterMode::MB_MASTER__REG_READ the application should copy
 *   the current values into the buffer \c pucRegBuffer.
 *
 * \return The function must return one of the following error codes:
 *   - eMB_MASTER_ErrorCode::MB_MASTER__ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMB_MASTER_ErrorCode::MB_MASTER__ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMB_MASTER_ErrorCode eMBMasterRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress,
		USHORT usNRegs, eMB_MASTER_RegisterMode eMode );

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Coil Register</em> value is
 *   read or written by the protocol stack. If you are going to use
 *   this function you might use the functions xMB_MASTER_UtilSetBits(  ) and
 *   xMB_MASTER_UtilGetBits(  ) for working with bitfields.
 *
 * \param pucRegBuffer The bits are packed in bytes where the first coil
 *   starting at address \c usAddress is stored in the LSB of the
 *   first byte in the buffer <code>pucRegBuffer</code>.
 *   If the buffer should be written by the callback function unused
 *   coil values (I.e. if not a multiple of eight coils is used) should be set
 *   to zero.
 * \param usAddress The first coil number.
 * \param usNCoils Number of coil values requested.
 * \param eMode If eMBRegisterMode::MB_REG_WRITE the application values should
 *   be updated from the values supplied in the buffer \c pucRegBuffer.
 *   If eMBRegisterMode::MB_REG_READ the application should store the current
 *   values in the buffer \c pucRegBuffer.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMB_MASTER_ErrorCode eMBMasterRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress,
		USHORT usNCoils, eMB_MASTER_RegisterMode eMode );

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Input Discrete Register</em> value is
 *   read by the protocol stack.
 *
 * If you are going to use his function you might use the functions
 * xMB_MASTER_UtilSetBits(  ) and xMB_MASTER_UtilGetBits(  ) for working with bitfields.
 *
 * \param pucRegBuffer The buffer should be updated with the current
 *   coil values. The first discrete input starting at \c usAddress must be
 *   stored at the LSB of the first byte in the buffer. If the requested nuMB_MASTER_er
 *   is not a multiple of eight the remaining bits should be set to zero.
 * \param usAddress The starting address of the first discrete input.
 * \param usNDiscrete Number of discrete input values.
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMB_MASTER_ErrorCode eMBMasterRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress,
		USHORT usNDiscrete );

/*! \ingroup modbus
 *\brief These Modbus functions are called for user when Modbus run in Master Mode.
 */
eMBMasterReqErrCode
eMBMasterReqReadInputRegister( UCHAR ucSndAddr, USHORT usRegAddr, USHORT usNRegs, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteHoldingRegister( UCHAR ucSndAddr, USHORT usRegAddr, USHORT usRegData, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteMultipleHoldingRegister( UCHAR ucSndAddr, USHORT usRegAddr,
		USHORT usNRegs, USHORT * pusDataBuffer, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadHoldingRegister( UCHAR ucSndAddr, USHORT usRegAddr, USHORT usNRegs, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadWriteMultipleHoldingRegister( UCHAR ucSndAddr,
		USHORT usReadRegAddr, USHORT usNReadRegs, USHORT * pusDataBuffer,
		USHORT usWriteRegAddr, USHORT usNWriteRegs, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadCoils( UCHAR ucSndAddr, USHORT usCoilAddr, USHORT usNCoils, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteCoil( UCHAR ucSndAddr, USHORT usCoilAddr, USHORT usCoilData, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteMultipleCoils( UCHAR ucSndAddr,
		USHORT usCoilAddr, USHORT usNCoils, UCHAR * pucDataBuffer, LONG lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadDiscreteInputs( UCHAR ucSndAddr, USHORT usDiscreteAddr, USHORT usNDiscreteIn, LONG lTimeOut );

eMB_MASTER_Exception
eMBMasterFuncReportSlaveID( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncReadInputRegister( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncReadHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncReadCoils( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncWriteCoil( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncWriteMultipleCoils( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen );
eMB_MASTER_Exception
eMBMasterFuncReadWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen );

/*£¡ \ingroup modbus
 *\brief These functions are interface for Modbus Master
 */
void vMBMasterGetPDUSndBuf( UCHAR ** pucFrame );
UCHAR ucMBMasterGetDestAddress( void );
void vMBMasterSetDestAddress( UCHAR Address );
BOOL xMBMasterGetCBRunInMasterMode( void );
void vMBMasterSetCBRunInMasterMode( BOOL IsMasterMode );
USHORT usMBMasterGetPDUSndLength( void );
void vMBMasterSetPDUSndLength( USHORT SendPDULength );
void vMBMasterSetCurTimerMode( eMBMasterTimerMode eMBTimerMode );
BOOL xMBMasterRequestIsBroadcast( void );
eMBMasterErrorEventType eMBMasterGetErrorType( void );
void vMBMasterSetErrorType( eMBMasterErrorEventType errorType );
eMBMasterReqErrCode eMBMasterWaitRequestFinish( void );

/* ----------------------- Callback -----------------------------------------*/

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
