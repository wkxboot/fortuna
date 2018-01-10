#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "device_status_task.h"


osThreadId device_status_task_hdl;
osMessageQId device_status_task_msg_q_id;

uint8_t ups_status,temperature;

void device_status_task(void const * argument)
{
  
while(1)
{
 osDelay(10);
}
  
  
  
}