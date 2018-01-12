/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define DOOR_RED_LED_CTL_POS_Pin GPIO_PIN_2
#define DOOR_RED_LED_CTL_POS_GPIO_Port GPIOE
#define DOOR_GREEN_LED_CTL_POS_Pin GPIO_PIN_3
#define DOOR_GREEN_LED_CTL_POS_GPIO_Port GPIOE
#define DOOR_ORANGE_LED_CTL_POS_Pin GPIO_PIN_4
#define DOOR_ORANGE_LED_CTL_POS_GPIO_Port GPIOE
#define T1_SENSOR_POS_Pin GPIO_PIN_6
#define T1_SENSOR_POS_GPIO_Port GPIOF
#define T2_SENSOR_POS_Pin GPIO_PIN_0
#define T2_SENSOR_POS_GPIO_Port GPIOC
#define AC1_CTL_POS_Pin GPIO_PIN_2
#define AC1_CTL_POS_GPIO_Port GPIOC
#define AC2_CTL_POS_Pin GPIO_PIN_3
#define AC2_CTL_POS_GPIO_Port GPIOC
#define DC12V2_CTL_POS_Pin GPIO_PIN_0
#define DC12V2_CTL_POS_GPIO_Port GPIOA
#define SYS_LED_CTL_POS_Pin GPIO_PIN_1
#define SYS_LED_CTL_POS_GPIO_Port GPIOA
#define DC12V1_CTL_POS_Pin GPIO_PIN_4
#define DC12V1_CTL_POS_GPIO_Port GPIOA
#define DOOR_STATUS_DWN_POS_Pin GPIO_PIN_5
#define DOOR_STATUS_DWN_POS_GPIO_Port GPIOA
#define LED1_CTL_POS_Pin GPIO_PIN_6
#define LED1_CTL_POS_GPIO_Port GPIOA
#define LED2_CTL_POS_Pin GPIO_PIN_7
#define LED2_CTL_POS_GPIO_Port GPIOA
#define GLASS_T_CTL_POS_Pin GPIO_PIN_4
#define GLASS_T_CTL_POS_GPIO_Port GPIOC
#define DISPLAY_COM6_CTL_POS_Pin GPIO_PIN_0
#define DISPLAY_COM6_CTL_POS_GPIO_Port GPIOG
#define DISPLAY_COM4_CTL_POS_Pin GPIO_PIN_1
#define DISPLAY_COM4_CTL_POS_GPIO_Port GPIOG
#define COMPRESSOR_CTL_POS_Pin GPIO_PIN_12
#define COMPRESSOR_CTL_POS_GPIO_Port GPIOE
#define DISPLAY_COM3_CTL_POS_Pin GPIO_PIN_12
#define DISPLAY_COM3_CTL_POS_GPIO_Port GPIOB
#define DISPLAY_COM5_CTL_POS_Pin GPIO_PIN_13
#define DISPLAY_COM5_CTL_POS_GPIO_Port GPIOB
#define W_T_SW_POS_Pin GPIO_PIN_14
#define W_T_SW_POS_GPIO_Port GPIOB
#define FUNC1_SW_POS_Pin GPIO_PIN_15
#define FUNC1_SW_POS_GPIO_Port GPIOB
#define ROW_SW_POS_Pin GPIO_PIN_8
#define ROW_SW_POS_GPIO_Port GPIOD
#define FUNC2_SW_POS_Pin GPIO_PIN_9
#define FUNC2_SW_POS_GPIO_Port GPIOD
#define DISPLAY_COM2_CTL_POS_Pin GPIO_PIN_15
#define DISPLAY_COM2_CTL_POS_GPIO_Port GPIOD
#define DISPLAY_COM1_CTL_POS_Pin GPIO_PIN_2
#define DISPLAY_COM1_CTL_POS_GPIO_Port GPIOG
#define DISPLAY_C_CTL_POS_Pin GPIO_PIN_3
#define DISPLAY_C_CTL_POS_GPIO_Port GPIOG
#define DISPLAY_E_CTL_POS_Pin GPIO_PIN_4
#define DISPLAY_E_CTL_POS_GPIO_Port GPIOG
#define DISPLAY_DP_CTL_POS_Pin GPIO_PIN_5
#define DISPLAY_DP_CTL_POS_GPIO_Port GPIOG
#define DISPLAY_D_CTL_POS_Pin GPIO_PIN_6
#define DISPLAY_D_CTL_POS_GPIO_Port GPIOG
#define DISPLAY_B_CTL_POS_Pin GPIO_PIN_7
#define DISPLAY_B_CTL_POS_GPIO_Port GPIOG
#define DISPLAY_A_CTL_POS_Pin GPIO_PIN_6
#define DISPLAY_A_CTL_POS_GPIO_Port GPIOC
#define DISPLAY_F_CTL_POS_Pin GPIO_PIN_7
#define DISPLAY_F_CTL_POS_GPIO_Port GPIOC
#define DISPLAY_G_CTL_POS_Pin GPIO_PIN_8
#define DISPLAY_G_CTL_POS_GPIO_Port GPIOC
#define LOCK_CTL_POS_Pin GPIO_PIN_11
#define LOCK_CTL_POS_GPIO_Port GPIOA
#define LOCK_SW_STATUS_POS_Pin GPIO_PIN_12
#define LOCK_SW_STATUS_POS_GPIO_Port GPIOA
#define DOOR_STATUS_UP_POS_Pin GPIO_PIN_0
#define DOOR_STATUS_UP_POS_GPIO_Port GPIOD
#define LOCK_STATUS_POS_Pin GPIO_PIN_1
#define LOCK_STATUS_POS_GPIO_Port GPIOE

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
 #define USE_FULL_ASSERT    1U 

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
