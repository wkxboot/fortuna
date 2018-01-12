#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "adc.h"
#include "ABDK_ZNHG_ZK.h"

#define  SYS_LED                      (1<<0)
#define  DOOR_RED_LED                 (1<<1)
#define  DOOR_GREEN_LED               (1<<2)
#define  DOOR_ORANGE_LED              (1<<3)
#define  LIGHT_1                      (1<<0)
#define  LIGHT_2                      (1<<1)
#define  AC_1                         (1<<0)
#define  AC_2                         (1<<1)
typedef enum 
{
/*LED*/
 LED_CTL_ON =GPIO_PIN_RESET,
 LED_CTL_OFF=GPIO_PIN_SET,
 LED_STATE_ON=GPIO_PIN_RESET,                     
 LED_STATE_OFF=GPIO_PIN_SET,
 /*交流电*/
 AC_CTL_ON=GPIO_PIN_SET,
 AC_CTL_OFF=GPIO_PIN_RESET,
 AC_STATE_ON=GPIO_PIN_SET,
 AC_STATE_OFF=GPIO_PIN_RESET,
 /*按键*/
 SW_STATE_PRESS=GPIO_PIN_RESET,
 SW_STATE_RELEASE=GPIO_PIN_SET,
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
 COMPRESSOR_PWR_STATE_OFF=GPIO_PIN_RESET
}bsp_state_t;

typedef enum 
{
/*在这里定义AC交流电状态值*/
 AC_ON=GPIO_PIN_RESET,                     
 AC_OFF=GPIO_PIN_SET 
}ac_state_t;

typedef enum 
{
/*在这里定义电插锁状态值*/
 LOCK_LOCK=GPIO_PIN_RESET,                     
 LOCK_UNLOCK=GPIO_PIN_SET 
}lock_state_t;

typedef enum 
{
/*在这里定义电插锁开关状态值*/
 SW_PRESS=GPIO_PIN_RESET,                     
 SW_RELEASE=GPIO_PIN_SET 
}sw_state_t;

typedef enum 
{
/*在这里定义门的状态值*/
 DOOR_OPEN=GPIO_PIN_RESET,                     
 DOOR_CLOSE=GPIO_PIN_SET 
}door_state_t;

typedef enum 
{
/*在这里定义玻璃加热电源状态值*/
 GLASS_PWR_ON=GPIO_PIN_RESET,                     
 GLASS_PWR_DWN=GPIO_PIN_SET 
}glass_pwr_state_t;

/*获取锁舌传感器状态*/
bsp_state_t BSP_get_lock_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(LOCK_STATUS_POS_GPIO_Port,LOCK_STATUS_POS_Pin);
}
/*获取开锁按键状态*/
bsp_state_t BSP_get_lock_sw_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(LOCK_SW_STATUS_POS_GPIO_Port,LOCK_SW_STATUS_POS_Pin);
 
}
/*获取门上部传感器状态*/
bsp_state_t BSP_get_door_up_status()
{
return (bsp_state_t)HAL_GPIO_ReadPin(DOOR_STATUS_UP_POS_GPIO_Port,DOOR_STATUS_UP_POS_Pin);
}
/*获取门下部传感器状态*/
bsp_state_t BSP_get_door_dwn_status()
{
return (bsp_state_t)HAL_GPIO_ReadPin(DOOR_STATUS_DWN_POS_GPIO_Port,DOOR_STATUS_DWN_POS_Pin);
}
/*获取W/T重量温度切换按键状态*/
bsp_state_t BSP_get_wt_sw_status()
{
return (bsp_state_t)HAL_GPIO_ReadPin(W_T_SW_POS_GPIO_Port,W_T_SW_POS_Pin);
}
/*获取货架层数切换按键状态*/
bsp_state_t BSP_get_row_sw_status()
{
return (bsp_state_t)HAL_GPIO_ReadPin(ROW_SW_POS_GPIO_Port,ROW_SW_POS_Pin);
}
/*获取功能按键1按键状态*/
bsp_state_t BSP_get_func1_sw_status()
{
return (bsp_state_t)HAL_GPIO_ReadPin(FUNC1_SW_POS_GPIO_Port,FUNC1_SW_POS_Pin);
}
/*获取功能按键2按键状态*/
bsp_state_t BSP_get_func2_sw_status()
{
return (bsp_state_t)HAL_GPIO_ReadPin(FUNC2_SW_POS_GPIO_Port,FUNC2_SW_POS_Pin);
}



/*led灯操作*/
void BSP_LED_TURN_ON_OFF(uint8_t led,bsp_state_t state )
{
 /*系统运行状态LED灯*/
 if(led & SYS_LED)
 {
 HAL_GPIO_WritePin(SYS_LED_CTL_POS_GPIO_Port,SYS_LED_CTL_POS_Pin,(GPIO_PinState)state); 
 }
/*门锁状态LED红灯*/
 if(led & DOOR_RED_LED)
 {
 HAL_GPIO_WritePin(DOOR_RED_LED_CTL_POS_GPIO_Port,DOOR_RED_LED_CTL_POS_Pin,(GPIO_PinState)state); 
 }
/*门锁状态LED绿灯*/
 if(led & DOOR_GREEN_LED)
 {
 HAL_GPIO_WritePin(DOOR_GREEN_LED_CTL_POS_GPIO_Port,DOOR_GREEN_LED_CTL_POS_Pin,(GPIO_PinState)state); 
 }
/*门锁状态LED橙色灯*/
 if(led & DOOR_ORANGE_LED)
 {
 HAL_GPIO_WritePin(DOOR_ORANGE_LED_CTL_POS_GPIO_Port,DOOR_ORANGE_LED_CTL_POS_Pin,(GPIO_PinState)state); 
 }
}

/*交流电操作*/
void BSP_TURN_ON_OFF_AC(uint8_t ac,bsp_state_t state)
{
/*交流电1*/
 if(ac & AC_1)
 {
 HAL_GPIO_WritePin(AC1_CTL_POS_GPIO_Port,AC1_CTL_POS_Pin,(GPIO_PinState)state); 
 }
/*交流电2*/
 if(ac & AC_2)
 {
 HAL_GPIO_WritePin(AC2_CTL_POS_GPIO_Port,AC2_CTL_POS_Pin,(GPIO_PinState)state); 
 }  
}
/*锁操作*/
void BSP_TURN_ON_OFF_LOCK(bsp_state_t state)
{
 HAL_GPIO_WritePin(LOCK_CTL_POS_GPIO_Port,LOCK_CTL_POS_Pin,(GPIO_PinState)state); 
}
/*玻璃加热电源操作*/
void BSP_TURN_ON_OFF_GLASS_PWR(bsp_state_t state)
{
 HAL_GPIO_WritePin(GLASS_T_CTL_POS_GPIO_Port,GLASS_T_CTL_POS_Pin,(GPIO_PinState)state);  
}


/*灯带操作*/
void BSP_TURN_ON_OFF_LIGHT(uint8_t light,bsp_state_t state)
{
  if(light & LIGHT_1)
  {
  HAL_GPIO_WritePin(LED1_CTL_POS_GPIO_Port,LED1_CTL_POS_Pin,(GPIO_PinState)state);  
  }
  if(light & LIGHT_2)
  {
  HAL_GPIO_WritePin(LED2_CTL_POS_GPIO_Port,LED2_CTL_POS_Pin,(GPIO_PinState)state);  
  }
  
}


