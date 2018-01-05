#ifndef    __MB_CB_M_H__
#define    __MB_CB_M_H__
/* ----------------------- Modbus includes ----------------------------------*/
#include "port_m.h"
#include "mbport_m.h"
#include "mbproto_m.h"
#include "mb_m.h"
#include "mbframe_m.h"
#include "mbconfig_m.h"
#include "mbutils_m.h"


/* -----------------------Master Defines -------------------------------------*/
#define  M_DISCRETE_INPUT_START        0
#define  M_DISCRETE_INPUT_NDISCRETES   1
#define  M_COIL_START                  0
#define  M_COIL_NCOILS                 8
#define  M_REG_INPUT_START             0
#define  M_REG_INPUT_NREGS             1
#define  M_REG_HOLDING_START           (40001)
#define  M_REG_HOLDING_NREGS           98

/*电子秤变送器寄存器地址*/
#define  DEVICE_ADDR_REG_ADDR              (40001)
#define  DEVICE_BAUDRATE_REG_ADDR          (40002)
#define  DEVICE_FSM_TYPE_REG_ADDR          (40003)
#define  DEVICE_PROTOCOL_TYPE_REG_ADDR     (40004)
#define  DEVICE_RESPONSE_DELAY_REG_ADDR    (40005)
#define  DEVICE_LOCK_REG_ADDR              (40006)
#define  DEVICE_FIRMWARE_VERTION_REG_ADDR  (40007)
#define  DEVICE_RESET_REG_ADDR             (40008)
#define  DEVICE_LOCK_REG_ADDR              (40009)

#define  DEVICE_MEASUREMENT_HI_REG_ADDR    (40031)
#define  DEVICE_MEASUREMENT_LO_REG_ADDR    (40032)
#define  DEVICE_ADC_SPEED_REG_ADDR         (40033)
#define  DEVICE_DIRECTION_REG_ADDR         (40034)
#define  DEVICE_SMOOTH_TYPE_REG_ADDR       (40035)
#define  DEVICE_SMOOTH_STRENGTH_REG_ADDR   (40036)
#define  DEVICE_ZERO_CODE_REG_ADDR         (40036)
#define  DEVICE_ZERO_MEASUREMENT_REG_ADDR  (40036)
#define  DEVICE_GAIN_CODE_REG_ADDR         (40036)
#define  DEVICE_GAIN_CODE_REG_ADDR         (40036)
#define  DEVICE_AD_ORIGIN_CODE_REG_ADDR    (40031)

#define  DEVICE_GROSS_WEIGHT_REG_ADDR      (40032)
#define  DEVICE_NET_WEIGHT_REG_ADDR        (40033)
#define  DEVICE_TARE_WEIGHT_REG_ADDR       (40034)
#define  DEVICE_MAX_WEIGHT_REG_ADDR        (40035)
#define  DEVICE_DIVISION_REG_ADDR          (40036)

#define  DEVICE_ZERO_CALIBRATE_HI_REG_ADDR (40032)
#define  DEVICE_ZERO_CALIBRATE_LO_REG_ADDR (40032)
#define  DEVICE_GAIN_CALIBRATE_HI_REG_ADDR (40033)
#define  DEVICE_GAIN_CALIBRATE_LO_REG_ADDR (40033)









/* master mode: holding register's all address */
#define          M_HD_RESERVE                     0
/* master mode: input register's all address */
#define          M_IN_RESERVE                     0
/* master mode: coil's all address */
#define          M_CO_RESERVE                     0
/* master mode: discrete's all address */
#define          M_DI_RESERVE                     0

#endif
