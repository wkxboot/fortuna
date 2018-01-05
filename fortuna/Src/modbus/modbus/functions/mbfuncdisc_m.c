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
#define MB_PDU_REQ_READ_DISCCNT_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_READ_SIZE                ( 4 )
#define MB_PDU_FUNC_READ_DISCCNT_OFF        ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READ_VALUES_OFF         ( MB_PDU_DATA_OFF + 1 )
#define MB_PDU_FUNC_READ_SIZE_MIN           ( 1 )

/* ----------------------- Static functions ---------------------------------*/
eMB_MASTER_Exception    prveMASTER_MBError2Exception( eMB_MASTER_ErrorCode eErrorCode );

/* ----------------------- Start implementation -----------------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0

/**
 * This function will request read discrete inputs.
 *
 * @param ucSndAddr salve address
 * @param usDiscreteAddr discrete start address
 * @param usNDiscreteIn discrete total number
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 */
eMBMasterReqErrCode
eMBMasterReqReadDiscreteInputs( UCHAR ucSndAddr, USHORT usDiscreteAddr, USHORT usNDiscreteIn, LONG lTimeOut )
{
    UCHAR                 *ucMBFrame;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
		vMBMasterGetPDUSndBuf(&ucMBFrame);
		vMBMasterSetDestAddress(ucSndAddr);
		ucMBFrame[MB_PDU_FUNC_OFF]                 = MB_FUNC_READ_DISCRETE_INPUTS;
		ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF]        = usDiscreteAddr >> 8;
		ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1]    = usDiscreteAddr;
		ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF ]    = usNDiscreteIn >> 8;
		ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF + 1] = usNDiscreteIn;
		vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_READ_SIZE );
		( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
		eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMB_MASTER_Exception
eMBMasterFuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usDiscreteCnt;
    UCHAR           ucNBytes;
    UCHAR          *ucMBFrame;

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

        usDiscreteCnt = ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF] << 8 );
        usDiscreteCnt |= ( USHORT )( ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF + 1] );

        /* Test if the quantity of coils is a multiple of 8. If not last
         * byte is only partially field with unused coils set to zero. */
        if( ( usDiscreteCnt & 0x0007 ) != 0 )
        {
        	ucNBytes = ( UCHAR )( usDiscreteCnt / 8 + 1 );
        }
        else
        {
        	ucNBytes = ( UCHAR )( usDiscreteCnt / 8 );
        }

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
		if ((usDiscreteCnt >= 1) && ucNBytes == pucFrame[MB_PDU_FUNC_READ_DISCCNT_OFF])
        {
	       	/* Make callback to fill the buffer. */
			eRegStatus = eMBMasterRegDiscreteCB( &pucFrame[MB_PDU_FUNC_READ_VALUES_OFF], usRegAddress, usDiscreteCnt );

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
#endif
