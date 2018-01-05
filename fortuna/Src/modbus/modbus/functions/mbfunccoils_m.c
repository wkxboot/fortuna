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
/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_REQ_READ_ADDR_OFF            ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_REQ_READ_COILCNT_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_READ_SIZE                ( 4 )
#define MB_PDU_FUNC_READ_COILCNT_OFF        ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READ_VALUES_OFF         ( MB_PDU_DATA_OFF + 1 )
#define MB_PDU_FUNC_READ_SIZE_MIN           ( 1 )

#define MB_PDU_REQ_WRITE_ADDR_OFF           ( MB_PDU_DATA_OFF )
#define MB_PDU_REQ_WRITE_VALUE_OFF          ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_WRITE_SIZE               ( 4 )
#define MB_PDU_FUNC_WRITE_ADDR_OFF          ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_WRITE_VALUE_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_SIZE              ( 4 )

#define MB_PDU_REQ_WRITE_MUL_ADDR_OFF       ( MB_PDU_DATA_OFF )
#define MB_PDU_REQ_WRITE_MUL_COILCNT_OFF    ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_WRITE_MUL_BYTECNT_OFF    ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_REQ_WRITE_MUL_VALUES_OFF     ( MB_PDU_DATA_OFF + 5 )
#define MB_PDU_REQ_WRITE_MUL_SIZE_MIN       ( 5 )
#define MB_PDU_REQ_WRITE_MUL_COILCNT_MAX    ( 0x07B0 )
#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF      ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF   ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_MUL_SIZE          ( 5 )

/* ----------------------- Static functions ---------------------------------*/
eMB_MASTER_Exception    prveMASTER_MBError2Exception( eMB_MASTER_ErrorCode eErrorCode );

/* ----------------------- Start implementation -----------------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
#if MB_FUNC_READ_COILS_ENABLED > 0

/**
 * This function will request read coil.
 *
 * @param ucSndAddr salve address
 * @param usCoilAddr coil start address
 * @param usNCoils coil total number
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 */
eMBMasterReqErrCode
eMBMasterReqReadCoils( UCHAR ucSndAddr, USHORT usCoilAddr, USHORT usNCoils ,LONG lTimeOut )
{
    UCHAR                 *ucMBFrame;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                 = MB_FUNC_READ_COILS;
		ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF]        = usCoilAddr >> 8;
		ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1]    = usCoilAddr;
		ucMBFrame[MB_PDU_REQ_READ_COILCNT_OFF ]    = usNCoils >> 8;
		ucMBFrame[MB_PDU_REQ_READ_COILCNT_OFF + 1] = usNCoils;
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_READ_SIZE );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );

    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncReadCoils( UCHAR * pucFrame, USHORT * usLen )
{
    UCHAR          *ucMBFrame;
    USHORT          usRegAddress;
    USHORT          usCoilCount;
    UCHAR           ucByteCount;

    eMB_MASTER_Exception    eStatus = MASTER_MB_EX_NONE;
    eMB_MASTER_ErrorCode    eRegStatus;

    /* If this request is broadcast, and it's read mode. This request don't need execute. */
    if ( xMBMasterRequestIsBroadcast() )
    {
    	eStatus = MASTER_MB_EX_NONE;
    }
    else if ( *usLen >= MB_PDU_SIZE_MIN + MB_PDU_FUNC_READ_SIZE_MIN )
    {
    	vMBMasterGetPDUSndBuf(&ucMBFrame);
        usRegAddress = ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1] );
        usRegAddress++;

        usCoilCount = ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_COILCNT_OFF] << 8 );
        usCoilCount |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_COILCNT_OFF + 1] );

        /* Test if the quantity of coils is a multiple of 8. If not last
         * byte is only partially field with unused coils set to zero. */
        if( ( usCoilCount & 0x0007 ) != 0 )
        {
        	ucByteCount = ( UCHAR )( usCoilCount / 8 + 1 );
        }
        else
        {
        	ucByteCount = ( UCHAR )( usCoilCount / 8 );
        }

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
        if( ( usCoilCount >= 1 ) &&
            ( ucByteCount == pucFrame[MB_PDU_FUNC_READ_COILCNT_OFF] ) )
        {
        	/* Make callback to fill the buffer. */
            eRegStatus = eMBMasterRegCoilsCB( &pucFrame[MB_PDU_FUNC_READ_VALUES_OFF], usRegAddress, usCoilCount, MB_MASTER_REG_READ );

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
        /* Can't be a valid read coil register request because the length
         * is incorrect. */
        eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
#endif

#if MB_FUNC_WRITE_COIL_ENABLED > 0

/**
 * This function will request write one coil.
 *
 * @param ucSndAddr salve address
 * @param usCoilAddr coil start address
 * @param usCoilData data to be written
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 *
 * @see eMBMasterReqWriteMultipleCoils
 */
eMBMasterReqErrCode
eMBMasterReqWriteCoil( UCHAR ucSndAddr, USHORT usCoilAddr, USHORT usCoilData, LONG lTimeOut )
{
    UCHAR                 *ucMBFrame;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( ( usCoilData != 0xFF00 ) && ( usCoilData != 0x0000 ) ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                = MB_FUNC_WRITE_SINGLE_COIL;
		ucMBFrame[MB_PDU_REQ_WRITE_ADDR_OFF]      = usCoilAddr >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_ADDR_OFF + 1]  = usCoilAddr;
		ucMBFrame[MB_PDU_REQ_WRITE_VALUE_OFF ]    = usCoilData >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_VALUE_OFF + 1] = usCoilData;
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_WRITE_SIZE );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncWriteCoil( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    UCHAR           ucBuf[2];

    eMB_MASTER_Exception    eStatus = MASTER_MB_EX_NONE;
    eMB_MASTER_ErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1] );
        usRegAddress++;

        if( ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF + 1] == 0x00 ) &&
            ( ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF ) ||
              ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0x00 ) ) )
        {
            ucBuf[1] = 0;
            if( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF )
            {
                ucBuf[0] = 1;
            }
            else
            {
                ucBuf[0] = 0;
            }
            eRegStatus =
                eMBMasterRegCoilsCB( &ucBuf[0], usRegAddress, 1, MB_MASTER_REG_WRITE );

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
        /* Can't be a valid write coil register request because the length
         * is incorrect. */
        eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif

#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0

/**
 * This function will request write multiple coils.
 *
 * @param ucSndAddr salve address
 * @param usCoilAddr coil start address
 * @param usNCoils coil total number
 * @param usCoilData data to be written
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 *
 * @see eMBMasterReqWriteCoil
 */
eMBMasterReqErrCode
eMBMasterReqWriteMultipleCoils( UCHAR ucSndAddr,
		USHORT usCoilAddr, USHORT usNCoils, UCHAR * pucDataBuffer, LONG lTimeOut)
{
    UCHAR                 *ucMBFrame;
    USHORT                 usRegIndex = 0;
    UCHAR                  ucByteCount;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( usNCoils > MB_PDU_REQ_WRITE_MUL_COILCNT_MAX ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                      = MB_FUNC_WRITE_MULTIPLE_COILS;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_ADDR_OFF]        = usCoilAddr >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_ADDR_OFF + 1]    = usCoilAddr;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_COILCNT_OFF]     = usNCoils >> 8;
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_COILCNT_OFF + 1] = usNCoils ;
		if( ( usNCoils & 0x0007 ) != 0 )
        {
			ucByteCount = ( UCHAR )( usNCoils / 8 + 1 );
        }
        else
        {
        	ucByteCount = ( UCHAR )( usNCoils / 8 );
        }
		ucMBFrame[MB_PDU_REQ_WRITE_MUL_BYTECNT_OFF]     = ucByteCount;
		ucMBFrame += MB_PDU_REQ_WRITE_MUL_VALUES_OFF;
		while( ucByteCount > usRegIndex)
		{
			*ucMBFrame++ = pucDataBuffer[usRegIndex++];
		}
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_WRITE_MUL_SIZE_MIN + ucByteCount );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncWriteMultipleCoils( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usCoilCnt;
    UCHAR           ucByteCount;
    UCHAR           ucByteCountVerify;
    UCHAR          *ucMBFrame;

    eMB_MASTER_Exception    eStatus = MASTER_MB_EX_NONE;
    eMB_MASTER_ErrorCode    eRegStatus;

    /* If this request is broadcast, the *usLen is not need check. */
    if( ( *usLen == MB_PDU_FUNC_WRITE_MUL_SIZE ) || xMBMasterRequestIsBroadcast() )
    {
    	vMBMasterGetPDUSndBuf(&ucMBFrame);
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1] );
        usRegAddress++;

        usCoilCnt = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF] << 8 );
        usCoilCnt |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF + 1] );

        ucByteCount = ucMBFrame[MB_PDU_REQ_WRITE_MUL_BYTECNT_OFF];

        /* Compute the number of expected bytes in the request. */
        if( ( usCoilCnt & 0x0007 ) != 0 )
        {
            ucByteCountVerify = ( UCHAR )( usCoilCnt / 8 + 1 );
        }
        else
        {
            ucByteCountVerify = ( UCHAR )( usCoilCnt / 8 );
        }

        if( ( usCoilCnt >= 1 ) && ( ucByteCountVerify == ucByteCount ) )
        {
            eRegStatus =
                eMBMasterRegCoilsCB( &ucMBFrame[MB_PDU_REQ_WRITE_MUL_VALUES_OFF],
                               usRegAddress, usCoilCnt, MB_MASTER_REG_WRITE );

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
        /* Can't be a valid write coil register request because the length
         * is incorrect. */
        eStatus = MASTER_MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif
#endif
