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
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port_m.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb_m.h"
#include "mbframe_m.h"

#if MB_MASTER_RTU_ENABLED == 1
#include "mbrtu_m.h"
#endif
#if MB_MASTER_ASCII_ENABLED == 1
#include "mbascii_m.h"
#endif
#if MB_MASTER_TCP_ENABLED == 1
#include "mbtcp_m.h"
#endif

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0

#ifndef MB_PORT_HAS_CLOSE
#define MB_PORT_HAS_CLOSE 0
#endif

/* ----------------------- Static variables ---------------------------------*/

static UCHAR    ucMBMasterDestAddress;
static BOOL     xMBRunInMasterMode = FALSE;
static eMBMasterErrorEventType eMBMasterCurErrorType;

static enum
{
    STATE_ENABLED,
    STATE_DISABLED,
    STATE_NOT_INITIALIZED
} eMBState = STATE_NOT_INITIALIZED;

/* Functions pointer which are initialized in eMBInit( ). Depending on the
 * mode (RTU or ASCII) the are set to the correct implementations.
 * Using for Modbus Master,Add by Armink 20130813
 */
static peMB_MASTER_FrameSend peMBMasterFrameSendCur;
static pvMB_MASTER_FrameStart pvMBMasterFrameStartCur;
static pvMB_MASTER_FrameStop pvMBMasterFrameStopCur;
static peMB_MASTER_FrameReceive peMBMasterFrameReceiveCur;
static pvMB_MASTER_FrameClose pvMBMasterFrameCloseCur;

/* Callback functions required by the porting layer. They are called when
 * an external event has happend which includes a timeout or the reception
 * or transmission of a character.
 * Using for Modbus Master,Add by Armink 20130813
 */
BOOL( *pxMBMasterFrameCBByteReceived ) ( void );
BOOL( *pxMBMasterFrameCBTransmitterEmpty ) ( void );
BOOL( *pxMBMasterPortCBTimerExpired ) ( void );

BOOL( *pxMBMasterFrameCBReceiveFSMCur ) ( void );
BOOL( *pxMBMasterFrameCBTransmitFSMCur ) ( void );

/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 */
static xMB_MASTER_FunctionHandler xMasterFuncHandlers[MB_FUNC_HANDLERS_MAX] = {
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
	//TODO Add Master function define
    {MB_FUNC_OTHER_REPORT_SLAVEID,eMB_MASTER_FuncReportSlaveID},
#endif
#if MB_FUNC_READ_INPUT_ENABLED > 0
    {MB_FUNC_READ_INPUT_REGISTER, eMBMasterFuncReadInputRegister},
#endif
#if MB_FUNC_READ_HOLDING_ENABLED > 0
    {MB_FUNC_READ_HOLDING_REGISTER, eMBMasterFuncReadHoldingRegister},
#endif
#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_REGISTERS, eMBMasterFuncWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_REGISTER, eMBMasterFuncWriteHoldingRegister},
#endif
#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
    {MB_FUNC_READWRITE_MULTIPLE_REGISTERS, eMBMasterFuncReadWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_READ_COILS_ENABLED > 0
    {MB_FUNC_READ_COILS, eMBMasterFuncReadCoils},
#endif
#if MB_FUNC_WRITE_COIL_ENABLED > 0
    {MB_FUNC_WRITE_SINGLE_COIL, eMBMasterFuncWriteCoil},
#endif
#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_COILS, eMBMasterFuncWriteMultipleCoils},
#endif
#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0
    {MB_FUNC_READ_DISCRETE_INPUTS, eMBMasterFuncReadDiscreteInputs},
#endif
};

/* ----------------------- Start implementation -----------------------------*/
eMB_MASTER_ErrorCode
eMBMasterInit(eMB_MASTER_Mode eMode,uint8_t port,uint32_t baudrate,uint8_t databits)
{
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;
    //eMB_MASTER_Mode          eMode  =MB_MASTER_RTU;
	switch (eMode)
	{
#if MB_MASTER_RTU_ENABLED > 0
	case MB_MASTER_RTU:
		pvMBMasterFrameStartCur = eMBMasterRTUStart;
		pvMBMasterFrameStopCur = eMBMasterRTUStop;
		peMBMasterFrameSendCur = eMBMasterRTUSend;
		peMBMasterFrameReceiveCur = eMBMasterRTUReceive;
		pvMBMasterFrameCloseCur = MB_PORT_HAS_CLOSE ? vMBMasterPortClose : NULL;
		pxMBMasterFrameCBByteReceived = xMBMasterRTUReceiveFSM;
		pxMBMasterFrameCBTransmitterEmpty = xMBMasterRTUTransmitFSM;
		pxMBMasterPortCBTimerExpired = xMBMasterRTUTimerExpired;

		eStatus = eMBMasterRTUInit(port,baudrate,databits);
		break;
#endif
#if MB_MASTER_ASCII_ENABLED > 0
		case MB_ASCII:
		pvMBMasterFrameStartCur = eMBMasterASCIIStart;
		pvMBMasterFrameStopCur = eMBMasterASCIIStop;
		peMBMasterFrameSendCur = eMBMasterASCIISend;
		peMBMasterFrameReceiveCur = eMBMasterASCIIReceive;
		pvMBMasterFrameCloseCur = MB_PORT_HAS_CLOSE ? vMBMasterPortClose : NULL;
		pxMBMasterFrameCBByteReceived = xMBMasterASCIIReceiveFSM;
		pxMBMasterFrameCBTransmitterEmpty = xMBMasterASCIITransmitFSM;
		pxMBMasterPortCBTimerExpired = xMBMasterASCIITimerT1SExpired;

		eStatus = eMBMasterASCIIInit(ucPort, ulBaudRate, eParity );
		break;
#endif
	default:
		eStatus = MB_MASTER_EINVAL;
		break;
	}

	if (eStatus == MB_MASTER_ENOERR)
	{
		if (!xMBMasterPortEventInit())
		{
			/* port dependent event module initalization failed. */
			eStatus = MB_MASTER_EPORTERR;
		}
		else
		{
			eMBState = STATE_DISABLED;
		}
		/* initialize the OS resource for modbus master. */
                if(!vMBMasterOsResInit())
                  eStatus = MB_MASTER_EPORTERR;
                  
	}
	return eStatus;
}

eMB_MASTER_ErrorCode
eMBMasterClose( void )
{
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;

    if( eMBState == STATE_DISABLED )
    {
        if( pvMBMasterFrameCloseCur != NULL )
        {
            pvMBMasterFrameCloseCur(  );
        }
    }
    else
    {
        eStatus = MB_MASTER_EILLSTATE;
    }
    return eStatus;
}

eMB_MASTER_ErrorCode
eMBMasterEnable( void )
{
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;

    if( eMBState == STATE_DISABLED )
    {
        /* Activate the protocol stack. */
        pvMBMasterFrameStartCur(  );
        eMBState = STATE_ENABLED;
    }
    else
    {
        eStatus = MB_MASTER_EILLSTATE;
    }
    return eStatus;
}

eMB_MASTER_ErrorCode
eMBMasterDisable( void )
{
    eMB_MASTER_ErrorCode    eStatus;

    if( eMBState == STATE_ENABLED )
    {
        pvMBMasterFrameStopCur(  );
        eMBState = STATE_DISABLED;
        eStatus = MB_MASTER_ENOERR;
    }
    else if( eMBState == STATE_DISABLED )
    {
        eStatus = MB_MASTER_ENOERR;
    }
    else
    {
        eStatus = MB_MASTER_EILLSTATE;
    }
    return eStatus;
}

eMB_MASTER_ErrorCode
eMBMasterPoll( void )
{
    static UCHAR   *ucMBFrame;
    static UCHAR    ucRcvAddress;
    static UCHAR    ucFunctionCode;
    static USHORT   usLength;
    static eMB_MASTER_Exception eException;

    int             i , j;
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;
    eMBMasterEventType    eEvent;
    eMBMasterErrorEventType errorType;

    /* Check if the protocol stack is ready. */
    if( eMBState != STATE_ENABLED )
    {
        return MB_MASTER_EILLSTATE;
    }

    /* Check if there is a event available. If not return control to caller.
     * Otherwise we will handle the event. */
    if( xMBMasterPortEventGet( &eEvent ) == TRUE )
    {
        switch ( eEvent )
        {
        case EV_MASTER_READY:
            break;

        case EV_MASTER_FRAME_RECEIVED:
			eStatus = peMBMasterFrameReceiveCur( &ucRcvAddress, &ucMBFrame, &usLength );
			/* Check if the frame is for us. If not ,send an error process event. */
			if ( ( eStatus == MB_MASTER_ENOERR ) && ( ucRcvAddress == ucMBMasterGetDestAddress() ) )
			{
				( void ) xMBMasterPortEventPost( EV_MASTER_EXECUTE );
			}
			else
			{
				vMBMasterSetErrorType(EV_ERROR_RECEIVE_DATA);
				( void ) xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
			}
			break;

        case EV_MASTER_EXECUTE:
            ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
            eException = MASTER_MB_EX_ILLEGAL_FUNCTION;
            /* If receive frame has exception .The receive function code highest bit is 1.*/
            if(ucFunctionCode >> 7) {
            	eException = (eMB_MASTER_Exception)ucMBFrame[MB_PDU_DATA_OFF];
            }
			else
			{
				for (i = 0; i < MB_FUNC_HANDLERS_MAX; i++)
				{
					/* No more function handlers registered. Abort. */
					if (xMasterFuncHandlers[i].ucFunctionCode == 0)	{
						break;
					}
					else if (xMasterFuncHandlers[i].ucFunctionCode == ucFunctionCode) {
						vMBMasterSetCBRunInMasterMode(TRUE);
						/* If master request is broadcast,
						 * the master need execute function for all slave.
						 */
						if ( xMBMasterRequestIsBroadcast() ) {
							usLength = usMBMasterGetPDUSndLength();
							for(j = 1; j <= MB_MASTER_TOTAL_SLAVE_NUM; j++){
								vMBMasterSetDestAddress(j);
								eException = xMasterFuncHandlers[i].pxHandler(ucMBFrame, &usLength);
							}
						}
						else {
							eException = xMasterFuncHandlers[i].pxHandler(ucMBFrame, &usLength);
						}
						vMBMasterSetCBRunInMasterMode(FALSE);
						break;
					}
				}
			}
            /* If master has exception ,Master will send error process.Otherwise the Master is idle.*/
            if (eException != MASTER_MB_EX_NONE) {
            	vMBMasterSetErrorType(EV_ERROR_EXECUTE_FUNCTION);
            	( void ) xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
            }
            else {
            	vMBMasterCBRequestSuccess( );
            	vMBMasterRunResRelease( );
            }
            break;

        case EV_MASTER_FRAME_SENT:
        	/* Master is busy now. */
        	vMBMasterGetPDUSndBuf( &ucMBFrame );
		eStatus = peMBMasterFrameSendCur( ucMBMasterGetDestAddress(), ucMBFrame, usMBMasterGetPDUSndLength() );
            break;

        case EV_MASTER_ERROR_PROCESS:
        	/* Execute specified error process callback function. */
			errorType = eMBMasterGetErrorType();
			vMBMasterGetPDUSndBuf( &ucMBFrame );
			switch (errorType) {
			case EV_ERROR_RESPOND_TIMEOUT:
				vMBMasterErrorCBRespondTimeout(ucMBMasterGetDestAddress(),
						ucMBFrame, usMBMasterGetPDUSndLength());
				break;
			case EV_ERROR_RECEIVE_DATA:
				vMBMasterErrorCBReceiveData(ucMBMasterGetDestAddress(),
						ucMBFrame, usMBMasterGetPDUSndLength());
				break;
			case EV_ERROR_EXECUTE_FUNCTION:
				vMBMasterErrorCBExecuteFunction(ucMBMasterGetDestAddress(),
						ucMBFrame, usMBMasterGetPDUSndLength());
				break;
			}
			vMBMasterRunResRelease();
        	break;
        }
    }
    return MB_MASTER_ENOERR;
}

/* Get whether the Modbus Master is run in master mode.*/
BOOL xMBMasterGetCBRunInMasterMode( void )
{
	return xMBRunInMasterMode;
}
/* Set whether the Modbus Master is run in master mode.*/
void vMBMasterSetCBRunInMasterMode( BOOL IsMasterMode )
{
	xMBRunInMasterMode = IsMasterMode;
}
/* Get Modbus Master send destination address. */
UCHAR ucMBMasterGetDestAddress( void )
{
	return ucMBMasterDestAddress;
}
/* Set Modbus Master send destination address. */
void vMBMasterSetDestAddress( UCHAR Address )
{
	ucMBMasterDestAddress = Address;
}
/* Get Modbus Master current error event type. */
eMBMasterErrorEventType eMBMasterGetErrorType( void )
{
	return eMBMasterCurErrorType;
}
/* Set Modbus Master current error event type. */
void vMBMasterSetErrorType( eMBMasterErrorEventType errorType )
{
	eMBMasterCurErrorType = errorType;
}



#endif
