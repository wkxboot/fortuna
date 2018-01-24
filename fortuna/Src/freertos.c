/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "fortuna_common.h"
#include "host_comm_task.h"
#include "scale_poll_task.h"
#include "scale_func_task.h"
#include "scale_comm_task.h"
#include "lock_task.h"
#include "display_led.h"
#include "switch_task.h"
#include "display_task.h"
#include "dc12v_task.h"
#include "light_task.h"
#include "sys_led_task.h"
#include "glass_pwr_task.h"
#include "temperature_task.h"
#include "compressor_task.h"
#include "glass_pwr_task.h"
#include "watch_dog_task.h"
#include "ups_task.h"
#include "weight_memory_task.h"
#include "temperature_memory_task.h"
#include "calibrate_memory_task.h"
#include "debug_task.h"
#define APP_LOG_MODULE_NAME   "[freertos]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */
EventGroupHandle_t task_sync_evt_group_hdl;
static void create_user_tasks();
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
__weak void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
  APP_LOG_ERROR("%s-任务栈溢出.\r\n",pcTaskName);
  APP_ERROR_HANDLER(0);
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
  APP_LOG_ERROR("系统任务内存分配失败.\r\n");
  APP_ERROR_HANDLER(0);
}
/* USER CODE END 5 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  /*创建用户任务*/
  APP_LOG_DEBUG("创建用户任务...\r\n");
  
  create_user_tasks();
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
static void create_user_tasks()
{
 /*创建任务同步事件组*/
  task_sync_evt_group_hdl=xEventGroupCreate();
  APP_ASSERT(task_sync_evt_group_hdl);
  
  /*创建主机通信任务*/
  osThreadDef(host_comm_task, host_comm_task, osPriorityNormal, 0, 128);
  host_comm_task_hdl = osThreadCreate(osThread(host_comm_task), NULL); 
  APP_ASSERT(host_comm_task_hdl);
  /*创建电子秤功能任务*/
  osThreadDef(scale_func_task, scale_func_task, osPriorityNormal, 0, 128);
  scale_func_task_hdl = osThreadCreate(osThread(scale_func_task), NULL); 
  APP_ASSERT(scale_func_task_hdl);
  /*创建电子秤轮询任务*/
  osThreadDef(scale_poll_task, scale_poll_task, osPriorityNormal, 0, 128);
  scale_poll_task_hdl = osThreadCreate(osThread(scale_poll_task), NULL); 
  APP_ASSERT(scale_poll_task_hdl);
  /*创建电子秤通信任务*/
  osThreadDef(scale_comm_task, scale_comm_task, osPriorityNormal, 0, 128);
  scale_comm_task_hdl = osThreadCreate(osThread(scale_comm_task), NULL); 
  APP_ASSERT(scale_comm_task_hdl);

  /*创建看门狗任务*/
  osThreadDef(watch_dog_task, watch_dog_task, osPriorityNormal, 0, 128);
  watch_dog_task_hdl = osThreadCreate(osThread(watch_dog_task), NULL); 
  APP_ASSERT(watch_dog_task_hdl);
  
  /*创建调试任务*/
  osThreadDef(debug_task, debug_task, osPriorityNormal, 0, 128);
  debug_task_hdl = osThreadCreate(osThread(debug_task), NULL); 
  APP_ASSERT(debug_task_hdl);
  
  
  /*创建锁任任务*/
  osThreadDef(lock_task, lock_task, osPriorityNormal, 0, 128);
  lock_task_hdl = osThreadCreate(osThread(lock_task), NULL); 
  APP_ASSERT(lock_task_hdl);
  /*创建数码管显示任务*/
  osThreadDef(display_task, display_task, osPriorityNormal, 0, 128);
  display_task_hdl = osThreadCreate(osThread(display_task), NULL); 
  APP_ASSERT(display_task_hdl);
  /*创建按键任务*/
  osThreadDef(switch_task,switch_task, osPriorityNormal, 0, 128);
  switch_task_hdl = osThreadCreate(osThread(switch_task), NULL); 
  APP_ASSERT(switch_task_hdl);
  /*创建压缩机任务*/
  osThreadDef(compressor_task, compressor_task, osPriorityNormal, 0, 128);
  compressor_task_hdl = osThreadCreate(osThread(compressor_task), NULL); 
  APP_ASSERT(compressor_task_hdl);
  /*创建系统LED任务*/
  osThreadDef(sys_led_task, sys_led_task, osPriorityNormal, 0, 128);
  sys_led_task_hdl = osThreadCreate(osThread(sys_led_task), NULL); 
  APP_ASSERT(sys_led_task_hdl);
  /*创建灯带任务*/
  osThreadDef(light_task, light_task, osPriorityNormal, 0, 128);
  light_task_hdl = osThreadCreate(osThread(light_task), NULL); 
  APP_ASSERT(light_task_hdl);
  /*创建12V任务*/
  osThreadDef(dc12v_task, dc12v_task, osPriorityNormal, 0, 128);
  dc12v_task_hdl = osThreadCreate(osThread(dc12v_task), NULL); 
  APP_ASSERT(dc12v_task_hdl);
  /*创建玻璃电源任务*/
  osThreadDef(glass_pwr_task, glass_pwr_task, osPriorityNormal, 0, 128);
  glass_pwr_task_hdl = osThreadCreate(osThread(glass_pwr_task), NULL); 
  APP_ASSERT(glass_pwr_task_hdl);
  /*创建温度任务*/
  osThreadDef(temperature_task, temperature_task, osPriorityNormal, 0, 128);
  temperature_task_hdl = osThreadCreate(osThread(temperature_task), NULL); 
  APP_ASSERT(temperature_task_hdl);
  /*创建UPS任务*/
  osThreadDef(ups_task, ups_task, osPriorityNormal, 0, 128);
  ups_task_hdl = osThreadCreate(osThread(ups_task), NULL); 
  APP_ASSERT(ups_task_hdl);
  
  /*创建重量缓存任务*/
  osThreadDef(weight_memory_task, weight_memory_task, osPriorityNormal, 0, 128);
  weight_memory_task_hdl = osThreadCreate(osThread(weight_memory_task), NULL); 
  APP_ASSERT(weight_memory_task_hdl);
    
  /*创建温度缓存任务*/
  osThreadDef(temperature_memory_task, temperature_memory_task, osPriorityNormal, 0, 128);
  temperature_memory_task_hdl = osThreadCreate(osThread(temperature_memory_task), NULL); 
  APP_ASSERT(temperature_memory_task_hdl);
  
/*创建校准缓存任务*/
  osThreadDef(calibrate_memory_task, calibrate_memory_task, osPriorityNormal, 0, 128);
  calibrate_memory_task_hdl = osThreadCreate(osThread(calibrate_memory_task), NULL); 
  APP_ASSERT(calibrate_memory_task_hdl);

  APP_LOG_INFO("所有任务创建成功.\r\n"); 
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
