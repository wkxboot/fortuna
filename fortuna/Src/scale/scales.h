#ifndef    __SCALES_H__
#define    __SCALES_H__
#include "fortuna_common.h"

fortuna_bool_t scale_get_net_weight(uint8_t sclae,uint16_t *ptr_net_weight);


/*称重单元数量*/
#define  SCALES_CNT_MAX                        4

/*无效的重量值*/
#define  SCLAE_NET_WEIGHT_INVALID_VALUE        0xffff

/*称重传感器寄存器地址*/
#define  M_DISCRETE_INPUT_START                0
#define  M_DISCRETE_INPUT_NDISCRETES           1
#define  M_COIL_START                          0
#define  M_COIL_NCOILS                         8
#define  M_REG_INPUT_START                     0
#define  M_REG_INPUT_NREGS                     1
#define  M_REG_HOLDING_START                   0
#define  M_REG_HOLDING_NREGS                   98

/*电子秤变送器寄存器地址*/
#define  DEVICE_ADDR_REG_ADDR                  0
#define  DEVICE_ADDR_REG_CNT                   1 
#define  DEVICE_BAUDRATE_REG_ADDR              1
#define  DEVICE_BAUDRATE_REG_CNT               1
#define  DEVICE_FSM_FORMAT_REG_ADDR            2
#define  DEVICE_FSM_FORMAT_REG_CNT             1
#define  DEVICE_PROTOCOL_FORMAT_REG_ADDR       3
#define  DEVICE_PROTOCOL_FORMAT_REG_CNT        1
#define  DEVICE_RESPONSE_DELAY_REG_ADDR        4
#define  DEVICE_RESPONSE_DELAY_REG_CNT         1
#define  DEVICE_LOCK_REG_ADDR                  5
#define  DEVICE_LOCK_REG_CNT                   1
#define  DEVICE_FIRMWARE_VERTION_REG_ADDR      6
#define  DEVICE_FIRMWARE_VERTION_REG_CNT       1
#define  DEVICE_RESET_REG_ADDR                 7
#define  DEVICE_RESET_REG_CNT                  1


#define  DEVICE_MEASUREMENT_REG_ADDR          30
#define  DEVICE_MEASUREMENT_REG_CNT            2
#define  DEVICE_ADC_SPEED_REG_ADDR            32
#define  DEVICE_ADC_SPEED_REG_CNT              1
#define  DEVICE_DIRECTION_REG_ADDR            33
#define  DEVICE_DIRECTION_REG_CNT              1
#define  DEVICE_SMOOTH_TYPE_REG_ADDR          34
#define  DEVICE_SMOOTH_TYPE_REG_CNT            1
#define  DEVICE_SMOOTH_STRENGTH_REG_ADDR      35
#define  DEVICE_SMOOTH_STRENGTH_REG_CNT        1
#define  DEVICE_ZERO_CODE_REG_ADDR            36
#define  DEVICE_ZERO_CODE_REG_CNT              2
#define  DEVICE_ZERO_MEASUREMENT_REG_ADDR     38
#define  DEVICE_ZERO_MEASUREMENT_REG_CNT       2
#define  DEVICE_SPAN_CODE_REG_ADDR            40
#define  DEVICE_SPAN_CODE_REG_CNT              2
#define  DEVICE_SPAN_MEASUREMENT_REG_ADDR     42
#define  DEVICE_SPAN_MEASUREMENT_REG_CNT       2
#define  DEVICE_AD_ORIGIN_CODE_REG_ADDR       44
#define  DEVICE_AD_ORIGIN_CODE_REG_CNT         2

#define  DEVICE_GROSS_WEIGHT_REG_ADDR         80
#define  DEVICE_GROSS_WEIGHT_REG_CNT           2
#define  DEVICE_NET_WEIGHT_REG_ADDR           82
#define  DEVICE_NET_WEIGHT_REG_CNT             2
#define  DEVICE_TARE_WEIGHT_REG_ADDR          84
#define  DEVICE_TARE_WEIGHT_REG_CNT            2
#define  DEVICE_MAX_WEIGHT_REG_ADDR           86
#define  DEVICE_MAX_WEIGHT_REG_CNT             2
#define  DEVICE_DIVISION_REG_ADDR             88
#define  DEVICE_DIVISION_REG_CNT               1
#define  DEVICE_ZERO_CALIBRATE_REG_ADDR       89
#define  DEVICE_ZERO_CALIBRATE_REG_CNT         2
#define  DEVICE_SPAN_CALIBRATE_REG_ADDR       91
#define  DEVICE_SPAN_CALIBRATE_REG_CNT         2
#define  DEVICE_MANUALLY_CLEAR_RANGE_REG_ADDR 93
#define  DEVICE_MANUALLY_CLEAR_RANGE_REG_CNT   1
#define  DEVICE_MANUALLY_CLEAR_REG_ADDR       94
#define  DEVICE_MANUALLY_CLEAR_REG_CNT         1
#define  DEVICE_PWR_ON_CLEAR_REG_ADDR         95
#define  DEVICE_PWR_ON_CLEAR_REG_CNT           1
#define  DEVICE_AUTO_CLEAR_RANGE_REG_ADDR     96
#define  DEVICE_AUTO_CLEAR_RANGE_REG_CNT       1
#define  DEVICE_AUTO_CLEAR_TIME_REG_ADDR      97
#define  DEVICE_AUTO_CLEAR_TIME_REG_CNT        1









/* master mode: holding register's all address */
#define          M_HD_RESERVE                     0
/* master mode: input register's all address */
#define          M_IN_RESERVE                     0
/* master mode: coil's all address */
#define          M_CO_RESERVE                     0
/* master mode: discrete's all address */
#define          M_DI_RESERVE                     0

#endif
