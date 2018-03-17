#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "string.h"
#include "json.h"
#include "http_get_post.h"
#include "shopping_task.h"
#include "device_task.h"

#define APP_LOG_MODULE_NAME   "[device]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


osThreadId device_report_task_hdl;

http_response_t http_response;
http_request_t http_request;

extern json_report_device_status_t  report_device;

/*设备上报任务*/
void device_report_task(void const * argument)
{
  app_bool_t result;
  uint8_t param_size;
  json_item_t item;
  while(1)
  {
  osDelay(DEVICE_TASK_INTERVAL);
  /*上报设备状态*/
  //get_ups_status();
  //get_temperature();
  while(1)
  {
  take_device_mutex();
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportDeviceStatus\"";
  if(json_body_to_str(&report_device,&http_request.ptr_param)!=APP_TRUE)
  {
  APP_LOG_ERROR("report param err.\r\n");
  }
  param_size=strlen((const char *)http_request.ptr_param);
  http_make_request_size_time_to_str(param_size,1000,http_request.size_time);
  
  result=http_post(&http_request,&http_response,100);
  if(result==APP_TRUE)
  {
  json_set_item_name_value(&item,"code",NULL);
  json_get_value_by_name_from_json_body(http_response.ptr_json,item.name,item.value); 
  if(result==APP_TRUE && strcmp((const char *)item.value,"\"0\"")==0)
  break;
  }
  release_device_mutex();
  osDelay(10);
  }  /*服务器回应code:"0"*/
  }  
}