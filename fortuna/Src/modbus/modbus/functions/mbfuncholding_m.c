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
#include "mbport_m.h"
#include "mb_m.h"
#include "mbframe_m.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_REQ_READ_ADDR_OFF                ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_REQ_READ_REGCNT_OFF              ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_READ_SIZE                    ( 4 )
#define MB_PDU_FUNC_READ_REGCNT_MAX             ( 0x007D )
#define MB_PDU_FUNC_READ_BYTECNT_OFF            ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READ_VALUES_OFF             ( MB_PDU_DATA_OFF + 1 )
#define MB_PDU_FUNC_READ_SIZE_MIN               ( 1 )

#define MB_PDU_REQ_WRITE_ADDR_OFF               ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_REQ_WRITE_VALUE_OFF              ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_WRITE_SIZE                   ( 4 )
#define MB_PDU_FUNC_WRITE_ADDR_OFF              ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_WRITE_VALUE_OFF             ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_SIZE                  ( 4 )

#define MB_PDU_REQ_WRITE_MUL_ADDR_OFF           ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_REQ_WRITE_MUL_REGCNT_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_WRITE_MUL_BYTECNT_OFF        ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_REQ_WRITE_MUL_VALUES_OFF         ( MB_PDU_DATA_OFF + 5 )
#define MB_PDU_REQ_WRITE_MUL_SIZE_MIN           ( 5 )
#define MB_PDU_REQ_WRITE_MUL_REGCNT_MAX         ( 0x0078 )
#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF          ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF        ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_MUL_SIZE              ( 4 )

#define MB_PDU_REQ_READWRITE_READ_ADDR_OFF      ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_REQ_READWRITE_READ_REGCNT_OFF    ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_READWRITE_WRITE_ADDR_OFF     ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_REQ_READWRITE_WRITE_REGCNT_OFF   ( MB_PDU_DATA_OFF + 6 )
#define MB_PDU_REQ_READWRITE_WRITE_BYTECNT_OFF  ( MB_PDU_DATA_OFF + 8 )
#define MB_PDU_REQ_READWRITE_WRITE_VALUES_OFF   ( MB_PDU_DATA_OFF + 9 )
#define MB_PDU_REQ_READWRITE_SIZE_MIN           ( 9 )
#define MB_PDU_FUNC_READWRITE_READ_BYTECNT_OFF  ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READWRITE_READ_VALUES_OFF   ( MB_PDU_DATA_OFF + 1 )
#define MB_PDU_FUNC_READWRITE_SIZE_MIN          ( 1 )

/* ----------------------- Static functions ---------------------------------*/
eMB_MASTER_Exception    prveMASTER_MBError2Exception( eMB_MASTER_ErrorCode eErrorCode );

/* ----------------------- Start implementation -----------------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0

/**
 * This function will request write holding register.
 *
 * @param ucSndAddr salve address
 * @param usRegAddr register start address
 * @param usRegData register data to be written
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 */
eMBMasterReqErrCode
eMBMasterReqWriteHoldingRegister( UCHAR ucSndAddr, USHORT usRegAddr, USHORT usRegData, LONG lTimeOut )
{
    UCHAR                 *ucMBFrame;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                = MB_FUNC_WRITE_REGISTER;
		ucMBFrame[MB_PDU_REQ_WRITE_ADDR_OFF]      = usRegAddr >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_ADDR_OFF + 1]  = usRegAddr;
		ucMBFrame[MB_PDU_REQ_WRITE_VALUE_OFF]     = usRegData >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_VALUE_OFF + 1] = usRegData ;
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_WRITE_SIZE );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    eMB_MASTER_Exception    eStatus = MASTER_MB_EX_NONE;
    eMB_MASTER_ErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_SIZE_MIN + MB_PDU_FUNC_WRITE_SIZE ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1] );
        usRegAddress++;

        /* Make callback to update the value. */
        eRegStatus = eMBMasterRegHoldingCB( &pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF],
                                      usRegAddress, 1, MB_MASTER_REG_WRITE );

        /* If an error occured convert it into a Modbus exception. */
        if( eRegStatus != MB_MASTER_ENOERR )
        {
            eStatus = prveMASTER_MBError2Exception( eRegStatus );
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
#endif

#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0

/**
 * This function will request write multiple holding register.
 *
 * @param ucSndAddr salve address
 * @param usRegAddr register start address
 * @param usNRegs register total number
 * @param pusDataBuffer data to be written
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 */
eMBMasterReqErrCode
eMBMasterReqWriteMultipleHoldingRegister( UCHAR ucSndAddr,
		USHORT usRegAddr, USHORT usNRegs, USHORT * pusDataBuffer, LONG lTimeOut )
{
    UCHAR                 *ucMBFrame;
    USHORT                 usRegIndex = 0;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                     = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_ADDR_OFF]       = usRegAddr >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_ADDR_OFF + 1]   = usRegAddr;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_REGCNT_OFF]     = usNRegs >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_REGCNT_OFF + 1] = usNRegs ;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_BYTECNT_OFF]    = usNRegs * 2;
		ucMBFrame += MB_PDU_REQ_WRITE_MUL_VALUES_OFF;
		while( usNRegs > usRegIndex)
		{
			*ucMBFrame++ = pusDataBuffer[usRegIndex] >> 8;
			*ucMBFrame++ = pusDataBuffer[usRegIndex++] ;
		}
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_WRITE_MUL_SIZE_MIN + 2*usNRegs );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    UCHAR          *ucMBFrame;
    USHORT          usRegAddress;
    USHORT          usRegCount;
    UCHAR           ucRegByteCount;

    eMB_MASTER_Exception    eStatus = MASTER_MB_EX_NONE;
    eMB_MASTER_ErrorCode    eRegStatus;

    /* If this request is broadcast, the *usLen is not need check. */
    if( ( *usLen == MB_PDU_SIZE_MIN + MB_PDU_FUNC_WRITE_MUL_SIZE ) || xMBMasterRequestIsBroadcast() )
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
        usRegAddress = ( USHORT )( ucMBFrame[MB_PDU_REQ_WRITE_MUL_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( ucMBFrame[MB_PDU_REQ_WRITE_MUL_ADDR_OFF + 1] );
        usRegAddress++;

        usRegCount = ( USHORT )( ucMBFrame[MB_PDU_REQ_WRITE_MUL_REGCNT_OFF] << 8 );
        usRegCount |= ( USHORT )( ucMBFrame[MB_PDU_REQ_WRITE_MUL_REGCNT_OFF + 1] );

        ucRegByteCount = ucMBFrame[MB_PDU_REQ_WRITE_MUL_BYTECNT_OFF];

        if( ucRegByteCount == 2 * usRegCount )
        {
            /* Make callback to update the register values. */
            eRegStatus =
                eMBMasterRegHoldingCB( &ucMBFrame[MB_PDU_REQ_WRITE_MUL_VALUES_OFF],
                                 usRegAddress, usRegCount, MB_MASTER_REG_WRITE );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_MASTER_ENOERR )
            {
                eStatus = prveMASTER_MBError2Exception( eRegStatus );
            }
        }
        else
        {
            eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
#endif

#if MB_FUNC_READ_HOLDING_ENABLED > 0

/**
 * This function will request read holding register.
 *
 * @param ucSndAddr salve address
 * @param usRegAddr register start address
 * @param usNRegs register total number
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 */
eMBMasterReqErrCode
eMBMasterReqReadHoldingRegister( UCHAR ucSndAddr, USHORT usRegAddr, USHORT usNRegs, LONG lTimeOut )
{
    UCHAR                 *ucMBFrame;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM )  eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                = MB_FUNC_READ_HOLDING_REGISTER;
		ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF]       = usRegAddr >> 8;
		ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1]   = usRegAddr;
		ucMBFrame[MB_PDU_REQ_READ_REGCNT_OFF]     = usNRegs >> 8;
		ucMBFrame[MB_PDU_REQ_READ_REGCNT_OFF + 1] = usNRegs;
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_READ_SIZE );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncReadHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    UCHAR          *ucMBFrame;
    USHORT          usRegAddress;
    USHORT          usRegCount;

    eMB_MASTER_Exception    eStatus = MASTER_MB_EX_NONE;
    eMB_MASTER_ErrorCode    eRegStatus;

    /* If this request is broadcast, and it's read mode. This request don't need execute. */
    if ( xMBMasterRequestIsBroadcast() )
    {
    	eStatus = MASTER_MB_EX_NONE;
    }
    else if( *usLen >= MB_PDU_SIZE_MIN + MB_PDU_FUNC_READ_SIZE_MIN )
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
        usRegAddress = ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1] );
        usRegAddress++;

        usRegCount = ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_REGCNT_OFF] << 8 );
        usRegCount |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_REGCNT_OFF + 1] );

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception.
         */
        if( ( usRegCount >= 1 ) && ( 2 * usRegCount == pucFrame[MB_PDU_FUNC_READ_BYTECNT_OFF] ) )
        {
            /* Make callback to fill the buffer. */
            eRegStatus = eMBMasterRegHoldingCB( &pucFrame[MB_PDU_FUNC_READ_VALUES_OFF], usRegAddress, usRegCount, MB_MASTER_REG_READ );
            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_MASTER_ENOERR )
            {
                eStatus = prveMASTER_MBError2Exception( eRegStatus );
            }
        }
        else
        {
            eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif

#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0

/**
 * This function will request read and write holding register.
 *
 * @param ucSndAddr salve address
 * @param usReadRegAddr read register start address
 * @param usNReadRegs read register total number
 * @param pusDataBuffer data to be written
 * @param usWriteRegAddr write register start address
 * @param usNWriteRegs write register total number
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 */
eMBMasterReqErrCode
eMBMasterReqReadWriteMultipleHoldingRegister( UCHAR ucSndAddr,
		USHORT usReadRegAddr, USHORT usNReadRegs, USHORT * pusDataBuffer,
		USHORT usWriteRegAddr, USHORT usNWriteRegs, LONG lTimeOut )
{
    UCHAR                 *ucMBFrame;
    USHORT                 usRegIndex = 0;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                           = MB_FUNC_READWRITE_MULTIPLE_REGISTERS;
		ucMBFrame[MB_PDU_REQ_READWRITE_READ_ADDR_OFF]        = usReadRegAddr >> 8;
		ucMBFrame[MB_PDU_REQ_READWRITE_READ_ADDR_OFF + 1]    = usReadRegAddr;
		ucMBFrame[MB_PDU_REQ_READWRITE_READ_REGCNT_OFF]      = usNReadRegs >> 8;
		ucMBFrame[MB_PDU_REQ_READWRITE_READ_REGCNT_OFF + 1]  = usNReadRegs ;
		ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_ADDR_OFF]       = usWriteRegAddr >> 8;
		ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_ADDR_OFF + 1]   = usWriteRegAddr;
		ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_REGCNT_OFF]     = usNWriteRegs >> 8;
		ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_REGCNT_OFF + 1] = usNWriteRegs ;
		ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_BYTECNT_OFF]    = usNWriteRegs * 2;
		ucMBFrame += MB_PDU_REQ_READWRITE_WRITE_VALUES_OFF;
		while( usNWriteRegs > usRegIndex)
		{
			*ucMBFrame++ = pusDataBuffer[usRegIndex] >> 8;
			*ucMBFrame++ = pusDataBuffer[usRegIndex++] ;
		}
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_READWRITE_SIZE_MIN + 2*usNWriteRegs );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncReadWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegReadAddress;
    USHORT          usRegReadCount;
    USHORT          usRegWriteAddress;
    USHORT          usRegWriteCount;
    UCHAR          *ucMBFrame;

    eMB_MASTER_Exception    eStatus = MASTER_MB_EX_NONE;
    eMB_MASTER_ErrorCode    eRegStatus;

    /* If this request is broadcast, and it's read mode. This request don't need execute. */
    if ( xMBMasterRequestIsBroadcast() )
    {
    	eStatus = MASTER_MB_EX_NONE;
    }
    else if( *usLen >= MB_PDU_SIZE_MIN + MB_PDU_FUNC_READWRITE_SIZE_MIN )
    {
    	vMBMasterGetPDUSndBuf(&ucMBFrame);
        usRegReadAddress = ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_READ_ADDR_OFF] << 8U );
        usRegReadAddress |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_READ_ADDR_OFF + 1] );
        usRegReadAddress++;

        usRegReadCount = ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_READ_REGCNT_OFF] << 8U );
        usRegReadCount |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_READ_REGCNT_OFF + 1] );

        usRegWriteAddress = ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_ADDR_OFF] << 8U );
        usRegWriteAddress |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_ADDR_OFF + 1] );
        usRegWriteAddress++;

        usRegWriteCount = ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_REGCNT_OFF] << 8U );
        usRegWriteCount |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_REGCNT_OFF + 1] );

        if( ( 2 * usRegReadCount ) == pucFrame[MB_PDU_FUNC_READWRITE_READ_BYTECNT_OFF] )
        {
            /* Make callback to update the register values. */
            eRegStatus = eMBMasterRegHoldingCB( &ucMBFrame[MB_PDU_REQ_READWRITE_WRITE_VALUES_OFF],
                                           usRegWriteAddress, usRegWriteCount, MB_MASTER_REG_WRITE );

            if( eRegStatus == MB_MASTER_ENOERR )
            {
                /* Make the read callback. */
				eRegStatus = eMBMasterRegHoldingCB(&pucFrame[MB_PDU_FUNC_READWRITE_READ_VALUES_OFF],
						                      usRegReadAddress, usRegReadCount, MB_MASTER_REG_READ);
            }
            if( eRegStatus != MB_MASTER_ENOERR )
            {
                eStatus = prveMASTER_MBError2Exception( eRegStatus );
            }
        }
        else
        {
            eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    return eStatus;
}

#endif
#endif

