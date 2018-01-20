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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"   
#include "mbconfig_m.h"
#include "mbutils_m.h"
#include "mbport_m.h"
#include "mb_m.h"
#include "scales.h"
#define APP_LOG_MODULE_NAME   "[scales]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"

/*-----------------------Master mode use these variables----------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0

//Master mode:DiscreteInputs variables
USHORT   usMDiscInStart                             = M_DISCRETE_INPUT_START;
#if      M_DISCRETE_INPUT_NDISCRETES%8
UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8+1];
#else
UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8];
#endif
//Master mode:Coils variables
USHORT   usMCoilStart                               = M_COIL_START;
#if      M_COIL_NCOILS%8
UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8+1];
#else
UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8];
#endif
//Master mode:InputRegister variables
USHORT   usMRegInStart                              = M_REG_INPUT_START;
USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];


//Master mode:HoldingRegister variables
USHORT   usMRegHoldStart                            = M_REG_HOLDING_START;
USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];


/*获取净重值*/
fortuna_bool_t get_net_weight(uint8_t scale,int32_t *ptr_net_weight)
{
  int32_t net_weight;
  uint8_t scale_start,scale_end;
  if(ptr_net_weight==NULL || scale > SCALES_CNT_MAX)
  return FORTUNA_FALSE;

 /*所有的称重量*/
  if(scale==0)
  {
  scale_start=1;
  scale_end=SCALES_CNT_MAX;
  }
  else
  {
  scale_start=scale;
  scale_end=scale_start; 
  }
  for(uint8_t i=scale_start;i<=scale_end;i++)
  {
  net_weight=usMRegHoldBuf[i-1][DEVICE_NET_WEIGHT_REG_ADDR]<<16|usMRegHoldBuf[i-1][DEVICE_NET_WEIGHT_REG_ADDR+1];
  *ptr_net_weight++=net_weight;
  }

 return FORTUNA_TRUE;
}

/**
 * Modbus master input register callback function.
 *
 * @param pucRegBuffer input register buffer
 * @param usAddress input register address
 * @param usNRegs input register number
 *
 * @return result
 */
eMB_MASTER_ErrorCode eMBMasterRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegInputBuf;
    USHORT          REG_INPUT_START;
    USHORT          REG_INPUT_NREGS;
    USHORT          usRegInStart;

    pusRegInputBuf = usMRegInBuf[ucMBMasterGetDestAddress() - 1];
    REG_INPUT_START = M_REG_INPUT_START;
    REG_INPUT_NREGS = M_REG_INPUT_NREGS;
    usRegInStart = usMRegInStart;

    /* it already plus one in modbus function method. */
    usAddress--;
    APP_LOG_INFO("MODBUS主机读输入寄存器 addr:%d size:%d\r\n",usAddress,usNRegs);
    
    if ((usAddress >= REG_INPUT_START)
            && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
    {
        iRegIndex = usAddress - usRegInStart;
        while (usNRegs > 0)
        {
          pusRegInputBuf[iRegIndex] = *pucRegBuffer++ << 8;
          pusRegInputBuf[iRegIndex] |= *pucRegBuffer++;
          iRegIndex++;
          usNRegs--;
        }
    }
    else
    {
        eStatus = MB_MASTER_ENOREG;
    }

    return eStatus;
}

/**
 * Modbus master holding register callback function.
 *
 * @param pucRegBuffer holding register buffer
 * @param usAddress holding register address
 * @param usNRegs holding register number
 * @param eMode read or write
 *
 * @return result
 */
eMB_MASTER_ErrorCode eMBMasterRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress,
        USHORT usNRegs, eMB_MASTER_RegisterMode eMode)
{
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegHoldingBuf;
    USHORT          REG_HOLDING_START;
    USHORT          REG_HOLDING_NREGS;
    USHORT          usRegHoldStart;

    pusRegHoldingBuf = usMRegHoldBuf[ucMBMasterGetDestAddress() - 1];
    REG_HOLDING_START = M_REG_HOLDING_START;
    REG_HOLDING_NREGS = M_REG_HOLDING_NREGS;
    usRegHoldStart = usMRegHoldStart;
    /* if mode is read, the master will write the received date to buffer. */
    eMode = MB_MASTER_REG_WRITE;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_HOLDING_START)
            && (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch (eMode)
        {
        /* read current register values from the protocol stack. */
        case MB_MASTER_REG_READ:
          APP_LOG_INFO("MODBUS主机读保持寄存器 addr:%d size:%d\r\n",usAddress,usNRegs);
            while (usNRegs > 0)
            {
              *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] >> 8);
              *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] & 0xFF);
              iRegIndex++;
              usNRegs--;
            }
            break;
        /* write current register values with new values from the protocol stack. */
        case MB_MASTER_REG_WRITE:
          APP_LOG_INFO("MODBUS主机写保持寄存器 addr:%d size:%d\r\n",usAddress,usNRegs);
            while (usNRegs > 0)
            {
              pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
              pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
              iRegIndex++;
              usNRegs--;
            }
            break;
        }
    }
    else
    {
        eStatus = MB_MASTER_ENOREG;
    }
    return eStatus;
}

/**
 * Modbus master coils callback function.
 *
 * @param pucRegBuffer coils buffer
 * @param usAddress coils address
 * @param usNCoils coils number
 * @param eMode read or write
 *
 * @return result
 */
eMB_MASTER_ErrorCode eMBMasterRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress,
        USHORT usNCoils, eMB_MASTER_RegisterMode eMode)
{
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucCoilBuf;
    USHORT          COIL_START;
    USHORT          COIL_NCOILS;
    USHORT          usCoilStart;
    iNReg =  usNCoils / 8 + 1;

    pucCoilBuf = ucMCoilBuf[ucMBMasterGetDestAddress() - 1];
    COIL_START = M_COIL_START;
    COIL_NCOILS = M_COIL_NCOILS;
    usCoilStart = usMCoilStart;

    /* if mode is read,the master will write the received date to buffer. */
    eMode = MB_MASTER_REG_WRITE;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= COIL_START)
            && (usAddress + usNCoils <= COIL_START + COIL_NCOILS))
    {
        iRegIndex = (USHORT) (usAddress - usCoilStart) / 8;
        iRegBitIndex = (USHORT) (usAddress - usCoilStart) % 8;
        switch (eMode)
        {
         /* read current coil values from the protocol stack. */
        case MB_MASTER_REG_READ:
            while (iNReg > 0)
            {
                *pucRegBuffer++ = xMB_MASTER_UtilGetBits(&pucCoilBuf[iRegIndex++],
                        iRegBitIndex, 8);
                iNReg--;
            }
            pucRegBuffer--;
            /* last coils */
            usNCoils = usNCoils % 8;
            /* filling zero to high bit */
            *pucRegBuffer = *pucRegBuffer << (8 - usNCoils);
            *pucRegBuffer = *pucRegBuffer >> (8 - usNCoils);
            break;

        /* write current coil values with new values from the protocol stack. */
        case MB_MASTER_REG_WRITE:
            while (iNReg > 1)
            {
                xMB_MASTER_UtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8,
                        *pucRegBuffer++);
                iNReg--;
            }
            /* last coils */
            usNCoils = usNCoils % 8;
            /* xMBUtilSetBits has bug when ucNBits is zero */
            if (usNCoils != 0)
            {
                xMB_MASTER_UtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils,
                        *pucRegBuffer++);
            }
            break;
        }
    }
    else
    {
        eStatus = MB_MASTER_ENOREG;
    }
    return eStatus;
}

/**
 * Modbus master discrete callback function.
 *
 * @param pucRegBuffer discrete buffer
 * @param usAddress discrete address
 * @param usNDiscrete discrete number
 *
 * @return result
 */
eMB_MASTER_ErrorCode eMBMasterRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMB_MASTER_ErrorCode    eStatus = MB_MASTER_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucDiscreteInputBuf;
    USHORT          DISCRETE_INPUT_START;
    USHORT          DISCRETE_INPUT_NDISCRETES;
    USHORT          usDiscreteInputStart;
    iNReg =  usNDiscrete / 8 + 1;

    pucDiscreteInputBuf = ucMDiscInBuf[ucMBMasterGetDestAddress() - 1];
    DISCRETE_INPUT_START = M_DISCRETE_INPUT_START;
    DISCRETE_INPUT_NDISCRETES = M_DISCRETE_INPUT_NDISCRETES;
    usDiscreteInputStart = usMDiscInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= DISCRETE_INPUT_START)
            && (usAddress + usNDiscrete    <= DISCRETE_INPUT_START + DISCRETE_INPUT_NDISCRETES))
    {
        iRegIndex = (USHORT) (usAddress - usDiscreteInputStart) / 8;
        iRegBitIndex = (USHORT) (usAddress - usDiscreteInputStart) % 8;

        /* write current discrete values with new values from the protocol stack. */
        while (iNReg > 1)
        {
            xMB_MASTER_UtilSetBits(&pucDiscreteInputBuf[iRegIndex++], iRegBitIndex, 8,
                    *pucRegBuffer++);
            iNReg--;
        }
        /* last discrete */
        usNDiscrete = usNDiscrete % 8;
        /* xMBUtilSetBits has bug when ucNBits is zero */
        if (usNDiscrete != 0)
        {
            xMB_MASTER_UtilSetBits(&pucDiscreteInputBuf[iRegIndex++], iRegBitIndex,
                    usNDiscrete, *pucRegBuffer++);
        }
    }
    else
    {
        eStatus = MB_MASTER_ENOREG;
    }

    return eStatus;
}
#endif

/*计算电子秤数量*/
void check_scale_cnt(uint8_t scale,uint8_t *ptr_scale_start,uint8_t *ptr_scale_end)
{
 if(scale==0)/*对所有的称*/
 {
  *ptr_scale_start=1;
  *ptr_scale_end=SCALES_CNT_MAX;
 }
 else
 {
  *ptr_scale_start=scale; 
  *ptr_scale_end= *ptr_scale_start;  
 }
}
/*电子秤手动清零范围设置*/
fortuna_bool_t scale_manully_zero_range(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 (void)scale_param;
 for(uint8_t i=0;i<DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT;i++)
 {
 param[i]=SCALE_ZERO_RANGE_VALUE>>((DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤设置手动置零范围...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MANUALLY_CLEAR_RANGE_REG_ADDR,DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤设置手动置零范围失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤设置手动置零范围成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}
/*电子秤清零*/
fortuna_bool_t scale_clear_zero(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_MANUALLY_CLEAR_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 (void)scale_param;
 for(uint8_t i=0;i<DEVICE_MANUALLY_CLEAR_REG_CNT;i++)
 {
 param[i]=SCALE_CLEAR_ZERO_VALUE>>((DEVICE_MANUALLY_CLEAR_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤清零...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MANUALLY_CLEAR_REG_ADDR,DEVICE_MANUALLY_CLEAR_REG_CNT,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤清零失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤清零成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤去皮/设置皮重值*/
fortuna_bool_t scale_remove_tare(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_TARE_WEIGHT_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);

 for(uint8_t i=0;i<DEVICE_TARE_WEIGHT_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_TARE_WEIGHT_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤去皮...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_TARE_WEIGHT_REG_ADDR,DEVICE_TARE_WEIGHT_REG_CNT,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤去皮失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤去皮成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}


/*电子秤标定内码值*/
fortuna_bool_t scale_calibrate_code(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_ZERO_CODE_REG_CNT];
 uint16_t reg_addr,reg_cnt;
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 
 if(scale_param==0)
 {
 reg_addr=DEVICE_ZERO_CODE_REG_ADDR;
 reg_cnt=DEVICE_ZERO_CODE_REG_CNT;
 APP_LOG_DEBUG("收到0点内码值标定消息.\r\n");
 }
 else
 {
 reg_addr=DEVICE_SPAN_CODE_REG_ADDR;
 reg_cnt=DEVICE_SPAN_CODE_REG_CNT;
 APP_LOG_DEBUG("收到非零点内码值标定消息.\r\n");
 }
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_ZERO_CODE_REG_CNT;i++)
 {
 param[i]=SCALE_AUTO_CODE_VALUE>>((DEVICE_ZERO_CODE_REG_CNT-1-i)*16);
 }
 
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤内码值标定...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,reg_addr,reg_cnt,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤内码值标定失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤内码值标定成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}


/*电子秤标定测量值*/
fortuna_bool_t scale_calibrate_measurement(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_ZERO_MEASUREMENT_REG_CNT];
 uint16_t reg_addr,reg_cnt;
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 
 if(scale_param==0)
 {
 reg_addr=DEVICE_ZERO_MEASUREMENT_REG_ADDR;
 reg_cnt=DEVICE_ZERO_MEASUREMENT_REG_CNT;
 APP_LOG_DEBUG("收到0点测量值标定消息.\r\n");
 }
 else
 {
 reg_addr=DEVICE_SPAN_MEASUREMENT_REG_ADDR;
 reg_cnt=DEVICE_SPAN_MEASUREMENT_REG_CNT;
 APP_LOG_DEBUG("收到非0点测量值标定消息.\r\n");
 }
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_ZERO_MEASUREMENT_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_ZERO_MEASUREMENT_REG_CNT-1-i)*16);
 }
 
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤测量值标定...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,reg_addr,reg_cnt,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤测量值标定失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤测量值标定成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤重量校准*/
fortuna_bool_t scale_calibrate_weight(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_ZERO_CALIBRATE_REG_CNT];
 uint16_t reg_addr,reg_cnt;
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 
 if(scale_param==0)
 {
 reg_addr=DEVICE_ZERO_CALIBRATE_REG_ADDR;
 reg_cnt=DEVICE_ZERO_CALIBRATE_REG_CNT;
 APP_LOG_DEBUG("收到0点重量值标定消息.\r\n");
 }
 else
 {
 reg_addr=DEVICE_SPAN_CALIBRATE_REG_ADDR;
 reg_cnt=DEVICE_SPAN_CALIBRATE_REG_CNT;
 APP_LOG_DEBUG("收到重量值标定消息.\r\n");
 }
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_ZERO_CALIBRATE_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_ZERO_CALIBRATE_REG_CNT-1-i)*16);
 }
 
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤重量值标定...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,reg_addr,reg_cnt,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤重量值标定失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤重量值标定成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤获取净重*/
fortuna_bool_t scale_obtain_net_weight(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 (void)scale_param;
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤获取净重...\r\n",scale);
  err_code=eMBMasterReqReadHoldingRegister(scale,DEVICE_NET_WEIGHT_REG_ADDR,DEVICE_NET_WEIGHT_REG_CNT,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤获取净重失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤获取净重成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤获取固件版本*/
fortuna_bool_t scale_obtain_firmware_version(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 (void)scale_param;
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤获取固件版本...\r\n",scale);
  err_code=eMBMasterReqReadHoldingRegister(scale,DEVICE_FIRMWARE_VERTION_REG_ADDR,DEVICE_FIRMWARE_VERTION_REG_CNT,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤获取固件版本失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤获取固件版本成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤设置最大称重值*/
fortuna_bool_t scale_set_max_weight(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_MAX_WEIGHT_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_MAX_WEIGHT_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_MAX_WEIGHT_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤设置最大称重值...\r\n",scale);
   err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_MAX_WEIGHT_REG_ADDR,DEVICE_MAX_WEIGHT_REG_CNT,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤设置最大称重值失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤设置最大称重值成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤设置分度值*/
fortuna_bool_t scale_set_division(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_DIVISION_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 for(uint8_t i=0;i<DEVICE_DIVISION_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_DIVISION_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤设置分度值...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_DIVISION_REG_ADDR,DEVICE_DIVISION_REG_CNT,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤设置分度值失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤设置分度值成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}

/*电子秤设备锁*/
fortuna_bool_t scale_lock_operation(uint8_t scale,uint32_t scale_param)
{
 uint8_t scale_end,scale_start;
 uint16_t param[DEVICE_LOCK_REG_CNT];
 fortuna_bool_t ret=FORTUNA_TRUE;
 eMBMasterReqErrCode err_code;
 /*计算电子秤数量*/
 check_scale_cnt(scale,&scale_start,&scale_end);
 if(scale_param==SCALE_UNLOCK_VALUE)
 {
 APP_LOG_DEBUG("解锁操作.\r\n");
 }
 else
 {
 APP_LOG_DEBUG("上锁操作.\r\n");
 }
 for(uint8_t i=0;i<DEVICE_LOCK_REG_CNT;i++)
 {
 param[i]=scale_param>>((DEVICE_LOCK_REG_CNT-1-i)*16);
 }
/*操作电子秤*/
 for(scale=scale_start;scale<=scale_end;scale++)
 {
  APP_LOG_DEBUG("%2d#电子秤锁操作...\r\n",scale);
  err_code=eMBMasterReqWriteMultipleHoldingRegister(scale,DEVICE_LOCK_REG_ADDR,DEVICE_LOCK_REG_CNT,param,SCALE_WAIT_TIMEOUT);
  if(err_code!=MB_MRE_NO_ERR)
  {
  ret=FORTUNA_FALSE;
  APP_LOG_ERROR("%2d#电子秤锁操作失败.\r\n",scale);
  return ret;/*只要有一个称出现错误就立即返回 减少响应时间*/
  }
  else
  {
  APP_LOG_DEBUG("%2d#电子秤锁操作成功.\r\n",scale);
  }
  osDelay(SCALE_OPERATION_INTERVAL);
 }
 return ret;
}



