#ifndef  __ABDK_ZNHG_ZK_H__
#define  __ABDK_ZNHG_ZK_H__
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "adc.h"


#define  CTL_TYPE_IO                     1
#define  CTL_TYPE_PWM                    2
#define  LIGHT_CTL_TYPE                  CTL_TYPE_PWM
#define  DC12V_2_CTL_TYPE                CTL_TYPE_PWM


#define  SYS_LED                        (1<<0)
#define  DOOR_RED_LED                   (1<<1)
#define  DOOR_GREEN_LED                 (1<<2)
#define  DOOR_ORANGE_LED                (1<<3)
#define  LIGHT_1                        (1<<0)
#define  LIGHT_2                        (1<<1)
#define  AC_1                           (1<<0)
#define  AC_2                           (1<<1)
#define  DC12V_1                        (1<<0)
#define  DC12V_2                        (1<<1)


typedef enum 
{
/*LED*/
 LED_CTL_ON =GPIO_PIN_RESET,
 LED_CTL_OFF=GPIO_PIN_SET,
 LED_STATE_ON=GPIO_PIN_RESET,                     
 LED_STATE_OFF=GPIO_PIN_SET,
 /*485 接收和发送使能控制*/
 RS485_RX_CTL_ENABLE=GPIO_PIN_RESET,
 RS485_TX_CTL_ENABLE=GPIO_PIN_SET,
 /*交流电*/
 AC_CTL_ON=GPIO_PIN_SET,
 AC_CTL_OFF=GPIO_PIN_RESET,
 AC_STATE_ON=GPIO_PIN_SET,
 AC_STATE_OFF=GPIO_PIN_RESET,
 /*按键*/
 SW_STATE_PRESS=GPIO_PIN_RESET,
 SW_STATE_RELEASE=GPIO_PIN_SET,
 /*UPS*/
 UPS_PWR_STATE_ON=GPIO_PIN_SET,/*UPS连接了主电源*/
 UPS_PWR_STATE_OFF=GPIO_PIN_RESET,/*UPS断开了主电源*/
 /*锁*/
 LOCK_CTL_OPEN=GPIO_PIN_SET,
 LOCK_CTL_CLOSE=GPIO_PIN_RESET,
 LOCK_STATE_OPEN=GPIO_PIN_SET,
 LOCK_STATE_CLOSE=GPIO_PIN_RESET,
 /*灯条*/
 LIGHT_CTL_ON=GPIO_PIN_SET,
 LIGHT_CTL_OFF=GPIO_PIN_RESET,
 LIGHT_STATE_ON=GPIO_PIN_SET,
 LIGHT_STATE_OFF=GPIO_PIN_RESET,
 /*玻璃温度*/
 GLASS_PWR_CTL_ON=GPIO_PIN_SET,
 GLASS_PWR_CTL_OFF=GPIO_PIN_RESET,
 GLASS_PWR_STATE_ON=GPIO_PIN_SET,
 GLASS_PWR_STATE_OFF=GPIO_PIN_RESET,
 /*压缩机*/
 COMPRESSOR_PWR_CTL_ON=GPIO_PIN_SET,
 COMPRESSOR_PWR_CTL_OFF=GPIO_PIN_RESET,
 COMPRESSOR_PWR_STATE_ON=GPIO_PIN_SET,
 COMPRESSOR_PWR_STATE_OFF=GPIO_PIN_RESET,
 /*12V输出控制*/
 DC12V_CTL_ON=GPIO_PIN_SET,
 DC12V_CTL_OFF=GPIO_PIN_RESET,
 DC12V_STATE_ON=GPIO_PIN_SET,
 DC12V_STATE_OFF=GPIO_PIN_RESET 
}bsp_state_t;

/*获取锁舌传感器状态*/
bsp_state_t BSP_get_lock_state();
/*获取开锁按键状态*/
bsp_state_t BSP_get_lock_sw_state();
/*获取UPS状态*/
bsp_state_t BSP_get_ups_state();
/*获取门上部传感器状态*/
bsp_state_t BSP_get_door_up_state();
/*获取门下部传感器状态*/
bsp_state_t BSP_get_door_dwn_state();
/*获取W/T重量温度切换按键状态*/
bsp_state_t BSP_get_wt_sw_state();
/*获取货架层数切换按键状态*/
bsp_state_t BSP_get_w_sw_state();
/*获取货校准按键按键状态*/
bsp_state_t BSP_get_calibrate_sw_state();
/*获取功能按键1按键状态*/
bsp_state_t BSP_get_func1_sw_state();
/*获取功能按键2按键状态*/
bsp_state_t BSP_get_func2_sw_state();
/*获取压缩机电源状态*/
bsp_state_t BSP_get_compressor_pwr_state();
/*获取玻璃加热电源状态*/
bsp_state_t BSP_get_glass_pwr_state();


/*led灯控制*/
void BSP_LED_TURN_ON_OFF(uint8_t led,bsp_state_t state);
/*RS485接收控制*/
void BSP_RS485_RX_ENABLE();
/*RS485发送控制*/
void BSP_RS485_TX_ENABLE();
/*交流电控制*/
void BSP_AC_TURN_ON_OFF(uint8_t ac,bsp_state_t state);
/*锁控制*/
void BSP_LOCK_TURN_ON_OFF(bsp_state_t state);
/*玻璃加热电源控制*/
void BSP_GLASS_PWR_TURN_ON_OFF(bsp_state_t state);
/*灯带控制*/
void BSP_LIGHT_TURN_ON_OFF(uint8_t light,bsp_state_t state);
/*压缩机控制*/
void BSP_COMPRESSOR_TURN_ON_OFF(bsp_state_t state);
/*12V输出控制*/
void BSP_DC12V_TURN_ON_OFF(uint8_t dc12,bsp_state_t state);















#endif