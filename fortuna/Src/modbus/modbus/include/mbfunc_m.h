#ifndef __MB_FUNC_M_H__
#define __MB_FUNC_M_H__

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif
#if MB_FUNC_OTHER_REP_SLAVEID_BUF > 0
eMB_MASTER_Exception    eMB_MASTER_FuncReportSlaveID( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_READ_INPUT_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncReadInputRegister( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_READ_HOLDING_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncReadHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_READ_COILS_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncReadCoils( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_WRITE_COIL_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncWriteCoil( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncWriteMultipleCoils( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen );
#endif

#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
eMB_MASTER_Exception    eMB_MASTER_FuncReadWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
#endif

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
