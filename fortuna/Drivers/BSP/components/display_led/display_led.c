#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "gpio.h"
#include "ABDK_ZNHG_ZK.h"
#include "display_led.h"


#if (DISPLAY_LED_TYPE == DISPLAY_LED_TYPE_COMMON_CATHODE)
static uint8_t const hex_code[]=
{
/*共阴数码管编码*/
/*0    1     2   3    4    5    6   7*/ 
0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
/*8    9     A   B    C    D    E   F*/
0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71
};
#elif (DISPLAY_LED_TYPE == DISPLAY_LED_TYPE_COMMON_ANODE)
static uint8_t const hex_code[]=
{
/*共阳数码管编码*/ 
/*0    1     2   3    4    5    6   7*/
0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,
/*8    9     A   B    C    D    E   F*/
0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e    
};
#else
#error "没有定义DISPLAY_LED_TYPE类型@DISPLAY_LED_TYPE_COMMON_CATHODE@DISPLAY_LED_TYPE_COMMON_ANODE"
#endif

/*在某个位置增加小数点*/
static uint8_t display_led_code_add_dp(uint8_t code)
{
 code+=DP_CODE;
 return code;
}
static void display_led_dis_off(uint8_t pos)
{
 if(pos>DIPPLAY_ALL_POS)
 pos=DIPPLAY_ALL_POS; 
 
  /*关闭需要的的显示*/
 if(pos & DISPLAY_LED_POS_1)
 HAL_GPIO_WritePin(DISPLAY_COM1_CTL_POS_GPIO_Port,DISPLAY_COM1_CTL_POS_Pin,DISPLAY_COM_OFF); 
 if(pos & DISPLAY_LED_POS_2)
 HAL_GPIO_WritePin(DISPLAY_COM2_CTL_POS_GPIO_Port,DISPLAY_COM2_CTL_POS_Pin,DISPLAY_COM_OFF);
 if(pos & DISPLAY_LED_POS_3)
 HAL_GPIO_WritePin(DISPLAY_COM3_CTL_POS_GPIO_Port,DISPLAY_COM3_CTL_POS_Pin,DISPLAY_COM_OFF);
 if(pos & DISPLAY_LED_POS_4)
 HAL_GPIO_WritePin(DISPLAY_COM4_CTL_POS_GPIO_Port,DISPLAY_COM4_CTL_POS_Pin,DISPLAY_COM_OFF);
 if(pos & DISPLAY_LED_POS_5)
 HAL_GPIO_WritePin(DISPLAY_COM5_CTL_POS_GPIO_Port,DISPLAY_COM5_CTL_POS_Pin,DISPLAY_COM_OFF);
 if(pos & DISPLAY_LED_POS_6)
 HAL_GPIO_WritePin(DISPLAY_COM6_CTL_POS_GPIO_Port,DISPLAY_COM6_CTL_POS_Pin,DISPLAY_COM_OFF);
}
static void display_led_dis_on(uint8_t pos)
{
 if(pos>DIPPLAY_ALL_POS)
 pos=DIPPLAY_ALL_POS;
 /*打开需要的显示*/
 if(pos & DISPLAY_LED_POS_1)
 HAL_GPIO_WritePin(DISPLAY_COM1_CTL_POS_GPIO_Port,DISPLAY_COM1_CTL_POS_Pin,DISPLAY_COM_ON); 
 if(pos & DISPLAY_LED_POS_2)
 HAL_GPIO_WritePin(DISPLAY_COM2_CTL_POS_GPIO_Port,DISPLAY_COM2_CTL_POS_Pin,DISPLAY_COM_ON);
 if(pos & DISPLAY_LED_POS_3)
 HAL_GPIO_WritePin(DISPLAY_COM3_CTL_POS_GPIO_Port,DISPLAY_COM3_CTL_POS_Pin,DISPLAY_COM_ON);
 if(pos & DISPLAY_LED_POS_4)
 HAL_GPIO_WritePin(DISPLAY_COM4_CTL_POS_GPIO_Port,DISPLAY_COM4_CTL_POS_Pin,DISPLAY_COM_ON);
 if(pos & DISPLAY_LED_POS_5)
 HAL_GPIO_WritePin(DISPLAY_COM5_CTL_POS_GPIO_Port,DISPLAY_COM5_CTL_POS_Pin,DISPLAY_COM_ON);
 if(pos & DISPLAY_LED_POS_6)
 HAL_GPIO_WritePin(DISPLAY_COM6_CTL_POS_GPIO_Port,DISPLAY_COM6_CTL_POS_Pin,DISPLAY_COM_ON);
}

/*数码管显示一位字符*/
static void display_led_dis_code(uint8_t code)
{
/*预先设置显示的字符*/
 HAL_GPIO_WritePin(DISPLAY_A_CTL_POS_GPIO_Port,DISPLAY_A_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_A)); 
 HAL_GPIO_WritePin(DISPLAY_B_CTL_POS_GPIO_Port,DISPLAY_B_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_B)); 
 HAL_GPIO_WritePin(DISPLAY_C_CTL_POS_GPIO_Port,DISPLAY_C_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_C)); 
 HAL_GPIO_WritePin(DISPLAY_D_CTL_POS_GPIO_Port,DISPLAY_D_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_D)); 
 HAL_GPIO_WritePin(DISPLAY_E_CTL_POS_GPIO_Port,DISPLAY_E_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_E)); 
 HAL_GPIO_WritePin(DISPLAY_F_CTL_POS_GPIO_Port,DISPLAY_F_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_F)); 
 HAL_GPIO_WritePin(DISPLAY_G_CTL_POS_GPIO_Port,DISPLAY_G_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_G)); 
 HAL_GPIO_WritePin(DISPLAY_DP_CTL_POS_GPIO_Port,DISPLAY_DP_CTL_POS_Pin,(GPIO_PinState)(code & DISPLAY_LED_DP)); 
 
}
/*在指定位置处显示1个数字*/
void display_led_dis_num(uint8_t pos,uint8_t num,fortuna_bool_t dp)
{
uint8_t code;
/*范围0-9-A-F*/
if(num==DISPLAY_LED_NULL_NUM)
{
 code = NULL_CODE;
}
else if(num==DISPLAY_LED_NEGATIVE_NUM)
{
 code=NEGATIVE_CODE; 
}
else if(num > 0x0f)
{
/*空字符加上小数点表示数字错误*/
code = NULL_CODE;
code =display_led_code_add_dp(code);
}
else
{
code=hex_code[num];
if(dp == FORTUNA_TRUE)
{
/*增加显示小数点*/
code=display_led_code_add_dp(code);
}
}
display_led_dis_off(DIPPLAY_ALL_POS);/*应该关闭所有的显示*/
display_led_dis_code(code);
display_led_dis_on(pos);
}


