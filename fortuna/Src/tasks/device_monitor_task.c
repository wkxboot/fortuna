#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "string.h"
#include "http.h"
#include "json.h"
#include "shopping_task.h"
#include "report_task.h"
#include "lock_task.h"
#include "ups_task.h"

#define APP_LOG_MODULE_NAME   "[monitor]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId device_monitor_task_hdl;

http_monitor_response monitor_response;



extern json_report_device_status_t  report_device;

/*设备上报任务*/
void device_monitor_task(void const * argument)
{
 app_bool_t result; 
 
init_handle:
  result=http_init();
  if(result!=APP_TRUE)
  {
   err_cnt++;
   if(err_cnt > DEVICE_MONITOR_TASK_RETRY_TIME_MAX)
   {
   APP_LOG_ERROR("错误次数超限.重启设备\r\n");
   err_cnt=0;
   BSP_GPRS_MODULE_RESET();
   osDelay(2000);
   BSP_GPRS_MODULE_WORK();
   }
  goto init_handle;
  }
  err_cnt=0;
  while(1)
  {
  result=http_device_status_monitor(&monitor_response);  
  if(result!=APP_TRUE)
  {
   err_cnt++;
   if(err_cnt > DEVICE_MONITOR_TASK_RETRY_TIME_MAX)
   {
   err_cnt=0;
   BSP_GPRS_MODULE_RESET();
   osDelay(2000);
   BSP_GPRS_MODULE_WORK();
   osDelay(5000);
   goto init_handle; 
   } 
  }
  
  
  
  
  osDelay(5000);
  }
  
}