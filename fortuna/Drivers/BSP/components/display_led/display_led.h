#ifndef  __DISPLAY_LED_H__
#define  __DISPLAY_LED_H__
#include "main.h"


#define  DISPLAY_LED_TYPE_COMMON_CATHODE     1
#define  DISPLAY_LED_TYPE_COMMON_ANODE       2

#define  DISPLAY_LED_TYPE                   DISPLAY_LED_TYPE_COMMON_CATHODE

#if (DISPLAY_LED_TYPE == DISPLAY_LED_TYPE_COMMON_CATHODE)/*共阴极*/
#define  DP_CODE                           (0x80)
#define  NULL_CODE                         (0x00)
#define  NEGATIVE_CODE                     (0x40)
#define  DISPLAY_COM_ON                    GPIO_PIN_RESET
#define  DISPLAY_COM_OFF                   GPIO_PIN_SET
#elif  (DISPLAY_LED_TYPE == DISPLAY_LED_TYPE_COMMON_ANODE)/*共阳极*/
#define  DP_CODE                           (-0x80)
#define  NULL_CODE                         (0xff)
#define  NEGATIVE_CODE                     (0xbf)
#define  DISPLAY_COM_ON                    GPIO_PIN_SET
#define  DISPLAY_COM_OFF                   GPIO_PIN_RESET
#else
#error "没有定义DISPLAY_LED_TYPE类型@DISPLAY_LED_TYPE_COMMON_CATHODE@DISPLAY_LED_TYPE_COMMON_ANODE"
#endif



#define  DISPLAY_LED_POS_1         (1<<0)
#define  DISPLAY_LED_POS_2         (1<<1)
#define  DISPLAY_LED_POS_3         (1<<2)
#define  DISPLAY_LED_POS_4         (1<<3)
#define  DISPLAY_LED_POS_5         (1<<4)
#define  DISPLAY_LED_POS_6         (1<<5)
#define  DIPPLAY_ALL_POS           ((1<<6)-1)

#define  DISPLAY_LED_A             (1<<0)
#define  DISPLAY_LED_B             (1<<1)
#define  DISPLAY_LED_C             (1<<2)
#define  DISPLAY_LED_D             (1<<3)
#define  DISPLAY_LED_E             (1<<4)
#define  DISPLAY_LED_F             (1<<5)
#define  DISPLAY_LED_G             (1<<6)
#define  DISPLAY_LED_DP            (1<<7)

#define  DISPLAY_LED_POS_CNT        6
#define  DISPLAY_LED_NULL_NUM       0xFF/*显示空白数字值*/
#define  DISPLAY_LED_NEGATIVE_NUM   0xFE/*显示负号*/

void display_led_dis_num(uint8_t pos,uint8_t num,fortuna_bool_t dp);









#endif