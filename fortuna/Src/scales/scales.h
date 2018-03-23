#ifndef    __SCALES_H__
#define    __SCALES_H__


app_bool_t get_net_weight(uint8_t sclae,int16_t *ptr_net_weight);

/*定义是否需要 净重读写操作互斥 目前不需要没问题*/
#define  SCALE_NET_WEIGHT_MUTEX_ENABLE                 1

/*电子秤相关操作的超时时间*/
#define  SCALE_NORMAL_WAIT_TIMEOUT                     20
#define  SCALE_NET_WEIGHT_WAIT_TIMEOUT                 20
#define  SCALE_REMOVE_TARE_WAIT_TIMEOUT                60
#define  SCALE_CLEAR_ZERO_WAIT_TIMEOUT                 100
#define  SCALE_CALIBRATE_WAIT_TIMEOUT                  120

#define  SCALE_LOCK_TIMEOUT                            80
#define  SCALE_NET_WEIGHT_MUXTEX_TIMEOUT               2


#define  SCALE_OPERATION_INTERVAL                      1


/*称重单元数量*/
#define  SCALES_CNT_MAX                                4

/*电子秤重量错误值定义*/
#define  SCALE_NET_WEIGHT_TIMEOUT_CNT                  10/*连续10次读不到称的重量数据就定义为超时*/

#define  SCALE_NET_WEIGHT_OVERLOAD_VALUE               30000
#define  SCALE_NET_WEIGHT_OVERLOAD_NEGATIVE_VALUE      (-3000)
#define  SCALE_NET_WEIGHT_OVERLOAD_CODE                (0x7FFF)

#define  SCALE_NET_WEIGHT_SPECIAL_NEGATIVE_VALUE       (-1)
#define  SCALE_NET_WEIGHT_SPECIAL_REPLACE_VALUE        0
#define  SCALE_NET_WEIGHT_ERR_CODE                     (-1)


/*电子秤操作值*/
#define  SCALE_UNLOCK_VALUE                            0x5AA5   
#define  SCALE_LOCK_VALUE                              0x0000/*其他任意值*/  
#define  SCALE_MAX_WEIGHT_VALUE                        30000/*30kg*/
#define  SCALE_DIVISION_VALUE                          0x0C/*1g*/
#define  SCALE_AUTO_TARE_WEIGHT_VALUE                  0x7fffffff
#define  SCALE_AUTO_CODE_VALUE                         0x7fffffff
#define  SCALE_CLEAR_ZERO_VALUE                        0x01                     
#define  SCALE_RESET_VALUE                             0x55
#define  SCALE_ZERO_RANGE_VALUE                        100/*%100最大量程值都可以手动清零*/
#define  SCALE_INVALID_WEIGHT_VALUE                    0xFFFF

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

/*电子秤初始化*/

#if SCALE_NET_WEIGHT_MUTEX_ENABLE > 0
void scale_init();
#endif

/*电子秤手动清零范围设置*/
app_bool_t scale_manully_zero_range(uint8_t scale,uint32_t scale_param);
/*电子秤清零操作*/
app_bool_t scale_clear_zero(uint8_t scale,uint32_t scale_param);
/*电子秤去皮操作*/
app_bool_t scale_remove_tare(uint8_t scale,uint32_t scale_param);

/*电子秤内码值校准操作*/
app_bool_t scale_calibrate_code(uint8_t scale,uint32_t scale_param);
/*电子秤测量值校准操作*/
app_bool_t scale_calibrate_measurement(uint8_t scale,uint32_t scale_param);
/*电子秤测量值校准操作*/
app_bool_t scale_calibrate_measurement(uint8_t scale,uint32_t scale_param);
/*电子秤重量校准操作*/
app_bool_t scale_calibrate_weight(uint8_t scale,uint32_t scale_param);
/*电子秤获取净重操作*/
app_bool_t scale_obtain_net_weight(uint8_t scale,uint32_t scale_param);
/*电子秤获取固件版本操作*/
app_bool_t scale_obtain_firmware_version(uint8_t scale,uint32_t scale_param);
/*电子秤设置最大称重值操作*/
app_bool_t scale_set_max_weight(uint8_t scale,uint32_t scale_param);
/*电子秤设置分度值操作*/
app_bool_t scale_set_division(uint8_t scale,uint32_t scale_param);
/*电子秤设备锁操作*/
app_bool_t scale_lock_operation(uint8_t scale,uint32_t scale_param);


#endif
