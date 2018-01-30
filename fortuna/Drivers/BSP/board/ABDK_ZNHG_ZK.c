#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "tim.h"
#include "ABDK_ZNHG_ZK.h"
#define APP_LOG_MODULE_NAME   "[BSP]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_ERROR    
#include "app_log.h"
#include "app_error.h"



/*获取锁舌传感器状态*/
bsp_state_t BSP_get_lock_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(LOCK_STATE_POS_GPIO_Port,LOCK_STATE_POS_Pin);
}
/*获取开锁按键状态*/
bsp_state_t BSP_get_lock_sw_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(LOCK_SW_STATE_POS_GPIO_Port,LOCK_SW_STATE_POS_Pin);
}

/*获取UPS 1状态*/
bsp_state_t BSP_get_ups1_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(UPS_1_STATE_POS_GPIO_Port,UPS_1_STATE_POS_Pin);
}
/*获取UPS 状态*/
bsp_state_t BSP_get_ups2_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(UPS_2_STATE_POS_GPIO_Port,UPS_2_STATE_POS_Pin);
}

/*获取门上部传感器状态*/
bsp_state_t BSP_get_door_up_state()
{
return (bsp_state_t)HAL_GPIO_ReadPin(DOOR_STATE_UP_POS_GPIO_Port,DOOR_STATE_UP_POS_Pin);
}
/*获取门下部传感器状态*/
bsp_state_t BSP_get_door_dwn_state()
{
return (bsp_state_t)HAL_GPIO_ReadPin(DOOR_STATE_DWN_POS_GPIO_Port,DOOR_STATE_DWN_POS_Pin);
}
/*获取W/T重量温度切换按键状态*/
bsp_state_t BSP_get_wt_sw_state()
{
return (bsp_state_t)HAL_GPIO_ReadPin(W_T_SW_STATE_POS_GPIO_Port,W_T_SW_STATE_POS_Pin);
}
/*获取货架层数切换按键状态*/
bsp_state_t BSP_get_w_sw_state()
{
return (bsp_state_t)HAL_GPIO_ReadPin(W_SW_STATE_POS_GPIO_Port,W_SW_STATE_POS_Pin);
}

/*获取校准按键按键状态*/
bsp_state_t BSP_get_calibrate_sw_state()
{
return (bsp_state_t)HAL_GPIO_ReadPin(CALIBRATE_SW_STATE_POS_GPIO_Port,CALIBRATE_SW_STATE_POS_Pin);
}
/*获取功能按键1按键状态*/
bsp_state_t BSP_get_func1_sw_state()
{
return (bsp_state_t)HAL_GPIO_ReadPin(FUNC1_SW_STATE_POS_GPIO_Port,FUNC1_SW_STATE_POS_Pin);
}
/*获取功能按键2按键状态*/
bsp_state_t BSP_get_func2_sw_state()
{
return (bsp_state_t)HAL_GPIO_ReadPin(FUNC2_SW_STATE_POS_GPIO_Port,FUNC2_SW_STATE_POS_Pin);
}

/*获取压缩机电源状态*/
bsp_state_t BSP_get_compressor_pwr_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(COMPRESSOR_CTL_POS_GPIO_Port,COMPRESSOR_CTL_POS_Pin);  
}

/*获取玻璃加热电源状态*/
bsp_state_t BSP_get_glass_pwr_state()
{
 return (bsp_state_t)HAL_GPIO_ReadPin(GLASS_T_CTL_POS_GPIO_Port,GLASS_T_CTL_POS_Pin); 
}


/*获取灯带状态--IO直接控制或者PWM控制*/
#if defined(LIGHT_CTL_TYPE) && (LIGHT_CTL_TYPE == CTL_TYPE_IO)/*IO直接控制*/ 
bsp_state_t BSP_get_light_state(uint8_t light)
{
 bsp_state_t state;
 if(light & LIGHT_1 && light & LIGHT_2)/*同时只能有一个灯获取状态。否则错误*/
 {
  APP_LOG_ERROR("同时获取2个灯带状态.\r\n");
  APP_ERROR_HANDLER(0);
 }
 if(light & LIGHT_1)
 {
 state= (bsp_state_t)HAL_GPIO_ReadPin(LED1_CTL_POS_GPIO_Port,LED1_CTL_POS_Pin);  
 }
 if(light & LIGHT_2)
 {
 state= (bsp_state_t)HAL_GPIO_ReadPin(LED2_CTL_POS_GPIO_Port,LED2_CTL_POS_Pin);  
 } 
 return state;
}
#elif defined(LIGHT_CTL_TYPE) && (LIGHT_CTL_TYPE == CTL_TYPE_PWM)/*PWM控制*/ 

static bsp_state_t bsp_light1_pwm_state=LIGHT_STATE_OFF;
static bsp_state_t bsp_light2_pwm_state=LIGHT_STATE_OFF;
bsp_state_t BSP_get_light_state(uint8_t light)
{
 if(light & LIGHT_1 && light & LIGHT_2)/*同时只能有一个灯获取状态。否则错误*/
 {
  APP_LOG_ERROR("同时获取2个灯带状态.\r\n")
  APP_ERROR_HANDLER(0);
 }
 
 if(light &LIGHT_1)
 {
  return bsp_light1_pwm_state;
 }
 if(light &LIGHT_2)
 {
  return bsp_light2_pwm_state;
 }
 return light1_pwm_state;/*不应该到这一步*/
}
#else 
#error "应该定义LIGHT_CTL_TYPE值"
#endif

/*led灯操作*/
void BSP_LED_TURN_ON_OFF(uint8_t led,bsp_state_t state)
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
void BSP_AC_TURN_ON_OFF(uint8_t ac,bsp_state_t state)
{
/*交流电1 交流风扇*/
 if(ac & AC_1)
 {
 HAL_GPIO_WritePin(AC1_CTL_POS_GPIO_Port,AC1_CTL_POS_Pin,(GPIO_PinState)state); 
 }
/*交流电2 交流GPU*/
 if(ac & AC_2)
 {
 HAL_GPIO_WritePin(AC2_CTL_POS_GPIO_Port,AC2_CTL_POS_Pin,(GPIO_PinState)state); 
 }  
}
/*锁操作*/
void BSP_LOCK_TURN_ON_OFF(bsp_state_t state)
{
 HAL_GPIO_WritePin(LOCK_CTL_POS_GPIO_Port,LOCK_CTL_POS_Pin,(GPIO_PinState)state); 
}
/*玻璃加热电源操作*/
void BSP_GLASS_PWR_TURN_ON_OFF(bsp_state_t state)
{
 HAL_GPIO_WritePin(GLASS_T_CTL_POS_GPIO_Port,GLASS_T_CTL_POS_Pin,(GPIO_PinState)state);  
}

/*灯带控制--IO控制或者PWM控制*/
#if defined(LIGHT_CTL_TYPE) && (LIGHT_CTL_TYPE == CTL_TYPE_IO)/*IO直接控制*/ 
void BSP_LIGHT_TURN_ON_OFF(uint8_t light,bsp_state_t state)
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
#elif defined(LIGHT_CTL_TYPE) && (LIGHT_CTL_TYPE == CTL_TYPE_PWM)/*PWM控制*/ 
void BSP_LIGHT_TURN_ON_OFF(uint8_t light,bsp_state_t state)
{
 if(light & LIGHT_1)
 {
 if(state==LIGHT_CTL_ON)
 {
 HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
 bsp_light1_pwm_state=LIGHT_STATE_ON;
 }
 else
 {
  /*强制PWM关闭时引脚电平为低，频率250HZ 占空比%80 所以把计数值设置在PWM输出无效值范围内（3200-3999）*/
 __HAL_TIM_SET_COUNTER(&htim3,3500);
 HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);
 bsp_light1_pwm_state=LIGHT_STATE_OFF;
 }
 }
 if(light & LIGHT_2)
 {
 if(state==LIGHT_CTL_ON)
 {
 HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2); 
 bsp_light2_pwm_state=LIGHT_STATE_ON;
 }
 else
 {
 /*强制PWM关闭时引脚电平为低，频率250HZ 占空比%80 所以把计数值设置在PWM输出无效值范围内（3200-3999）*/
 __HAL_TIM_SET_COUNTER(&htim3,3500);
 HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_2);
 bsp_light2_pwm_state=LIGHT_STATE_OFF;
 }
 }
}
#else
#error "应该定义LIGHT_CTL_TYPE值."
#endif
/*压缩机控制*/
void BSP_COMPRESSOR_TURN_ON_OFF(bsp_state_t state)
{
 HAL_GPIO_WritePin(COMPRESSOR_CTL_POS_GPIO_Port,COMPRESSOR_CTL_POS_Pin,(GPIO_PinState)state);  
}

/*12V输出控制--IO控制或者PWM控制*/
#if defined(DC12V_2_CTL_TYPE) && (DC12V_2_CTL_TYPE == CTL_TYPE_IO)/*IO直接控制*/  
void BSP_DC12V_TURN_ON_OFF(uint8_t dc12,bsp_state_t state)
{
 if(dc12 & DC12V_1)
 {
 HAL_GPIO_WritePin(DC12V1_CTL_POS_GPIO_Port,DC12V1_CTL_POS_Pin,(GPIO_PinState)state);   
 }
 if(dc12 & DC12V_2)
 { 
 HAL_GPIO_WritePin(DC12V2_CTL_POS_GPIO_Port,DC12V2_CTL_POS_Pin,(GPIO_PinState)state);   
 
 }
 
}
#elif defined(DC12V_2_CTL_TYPE) && (DC12V_2_CTL_TYPE == CTL_TYPE_PWM)/*PWM控制*/
void BSP_DC12V_TURN_ON_OFF(uint8_t dc12,bsp_state_t state)
{
 if(dc12 & DC12V_1)
 {
 HAL_GPIO_WritePin(DC12V1_CTL_POS_GPIO_Port,DC12V1_CTL_POS_Pin,(GPIO_PinState)state);   
 }
 if(state==DC12V_CTL_ON)
 HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_1); 
 else
 {
 /*强制PWM关闭时引脚电平为低，频率1000HZ 占空比%50 所以把计数值设置在PWM输出无效值范围内（500-999）*/
 __HAL_TIM_SET_COUNTER(&htim5,700); 
 HAL_TIM_PWM_Stop(&htim5,TIM_CHANNEL_1); 
 }
} 
#else
#error "应该定义DC12V_2_CTL_TYPE值."
#endif 



