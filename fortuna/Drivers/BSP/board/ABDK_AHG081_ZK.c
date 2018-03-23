#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "ABDK_AHG081_ZK.h"
#define APP_LOG_MODULE_NAME   "[BSP]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_ERROR    
#include "app_log.h"
#include "app_error.h"





/*获取锁舌传感器状态*/
bsp_status_t BSP_get_lock_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(LOCK_STATUS_POS_GPIO_Port,LOCK_STATUS_POS_Pin);
}
/*获取开锁按键状态*/
bsp_status_t BSP_get_lock_sw_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(LOCK_SW_STATUS_POS_GPIO_Port,LOCK_SW_STATUS_POS_Pin);
}

/*获取UPS 1状态*/
bsp_status_t BSP_get_ups1_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(UPS_1_STATUS_POS_GPIO_Port,UPS_1_STATUS_POS_Pin);
}
/*获取UPS 状态*/
bsp_status_t BSP_get_ups2_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(UPS_2_STATUS_POS_GPIO_Port,UPS_2_STATUS_POS_Pin);
}

/*获取门上部传感器状态*/
bsp_status_t BSP_get_door_up_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(DOOR_STATUS_UP_POS_GPIO_Port,DOOR_STATUS_UP_POS_Pin);
}
/*获取门下部传感器状态*/
bsp_status_t BSP_get_door_dwn_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(DOOR_STATUS_DWN_POS_GPIO_Port,DOOR_STATUS_DWN_POS_Pin);
}
/*获取W/T重量温度切换按键状态*/
bsp_status_t BSP_get_wt_sw_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(W_T_SW_STATUS_POS_GPIO_Port,W_T_SW_STATUS_POS_Pin);
}
/*获取货架层数切换按键状态*/
bsp_status_t BSP_get_w_sw_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(W_SW_STATUS_POS_GPIO_Port,W_SW_STATUS_POS_Pin);
}

/*获取校准按键按键状态*/
bsp_status_t BSP_get_calibrate_sw_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(CALIBRATE_SW_STATUS_POS_GPIO_Port,CALIBRATE_SW_STATUS_POS_Pin);
}
/*获取功能按键1按键状态*/
bsp_status_t BSP_get_func1_sw_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(FUNC1_SW_STATUS_POS_GPIO_Port,FUNC1_SW_STATUS_POS_Pin);
}
/*获取功能按键2按键状态*/
bsp_status_t BSP_get_func2_sw_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(FUNC2_SW_STATUS_POS_GPIO_Port,FUNC2_SW_STATUS_POS_Pin);
}

/*获取压缩机电源状态*/
static bsp_status_t bsp_compressor_pwr_status=COMPRESSOR_PWR_STATUS_OFF;
bsp_status_t BSP_get_compressor_pwr_status()
{
 return bsp_compressor_pwr_status;  
}

/*获取玻璃加热电源状态*/
static bsp_status_t bsp_glass_pwr_status=GLASS_PWR_STATUS_OFF;
bsp_status_t BSP_get_glass_pwr_status()
{
 return bsp_glass_pwr_status;
}


/*灯带状态*/
static bsp_status_t bsp_light_status=LIGHT_STATUS_ON;
/*风扇状态*/
static bsp_status_t bsp_fan_status=LIGHT_STATUS_ON;

bsp_status_t BSP_get_light_status()
{
 return bsp_light_status;
}
bsp_status_t BSP_fan_light_status()
{
 return bsp_fan_status;
}
/*led灯操作*/
void BSP_LED_TURN_ON_OFF(uint8_t led,bsp_status_t status)
{
 /*系统运行状态LED灯*/
 if(led & SYS_LED)
 {
 HAL_GPIO_WritePin(SYS_LED_CTL_POS_GPIO_Port,SYS_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
/*门锁状态LED红灯*/
 if(led & DOOR_RED_LED)
 {
 HAL_GPIO_WritePin(DOOR_RED_LED_CTL_POS_GPIO_Port,DOOR_RED_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
/*门锁状态LED绿灯*/
 if(led & DOOR_GREEN_LED)
 {
 HAL_GPIO_WritePin(DOOR_GREEN_LED_CTL_POS_GPIO_Port,DOOR_GREEN_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
/*门锁状态LED橙色灯*/
 if(led & DOOR_ORANGE_LED)
 {
 HAL_GPIO_WritePin(DOOR_ORANGE_LED_CTL_POS_GPIO_Port,DOOR_ORANGE_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
}
/*RS485接收和发送控制*/
void BSP_RS485_RX_ENABLE()
{
 HAL_GPIO_WritePin(RS485_RT_CTL_POS_GPIO_Port,RS485_RT_CTL_POS_Pin,(GPIO_PinState)RS485_RX_CTL_ENABLE); 
}
void BSP_RS485_TX_ENABLE()
{
 HAL_GPIO_WritePin(RS485_RT_CTL_POS_GPIO_Port,RS485_RT_CTL_POS_Pin,(GPIO_PinState)RS485_TX_CTL_ENABLE);  
}
/*交流电操作*/
void BSP_AC_TURN_ON_OFF(uint8_t ac,bsp_status_t status)
{
/*交流电1 交流GPU*/
 if(ac & AC_1)
 {
 HAL_GPIO_WritePin(AC1_CTL_POS_GPIO_Port,AC1_CTL_POS_Pin,(GPIO_PinState)status); 
 }
/*交流电2 交流风扇*/
 if(ac & AC_2)
 {
 HAL_GPIO_WritePin(AC2_CTL_POS_GPIO_Port,AC2_CTL_POS_Pin,(GPIO_PinState)status); 
 }  
}
/*锁操作*/
void BSP_LOCK_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(LOCK_CTL_POS_GPIO_Port,LOCK_CTL_POS_Pin,(GPIO_PinState)status); 
}
/*玻璃加热电源操作*/
void BSP_GLASS_PWR_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(GLASS_PWR_CTL_POS_GPIO_Port,GLASS_PWR_CTL_POS_Pin,(GPIO_PinState)status);
 bsp_glass_pwr_status=status;
}

/*灯带控制--PWM控制*/
void BSP_LIGHT_TURN_ON_OFF(bsp_status_t status)
{
 bsp_light_status=status;
 if(status==LIGHT_CTL_ON)
 {
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);  
 }
 else
 {
 HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_3); 
 }
}
/*直流风扇控制--IO控制*/
void BSP_FAN_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(FAN_CTL_POS_GPIO_Port,FAN_CTL_POS_Pin,(GPIO_PinState)status); 
 bsp_fan_status=status; 
}

/*压缩机控制*/
void BSP_COMPRESSOR_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(COMPRESSOR_CTL_POS_GPIO_Port,COMPRESSOR_CTL_POS_Pin,(GPIO_PinState)status);  
 bsp_compressor_pwr_status=status;
}

/*12V输出控制--IO控制*/
void BSP_DC12V_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(DC12V_CTL_POS_GPIO_Port,DC12V_CTL_POS_Pin,(GPIO_PinState)status);   
}

/*24V输出控制--IO控制*/
void BSP_DC24V_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(DC24V_CTL_POS_GPIO_Port,DC24V_CTL_POS_Pin,(GPIO_PinState)status);   
}


