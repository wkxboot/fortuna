#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "string.h"
#include "json.h"
#include "http_get_post.h"
#include "shopping_task.h"
#include "report_task.h"

#define APP_LOG_MODULE_NAME   "[device]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


osThreadId report_task_hdl;

static http_response_t http_response;
static http_request_t  http_request;

extern json_report_device_status_t  report_device;

/*设备上报任务*/
void report_task(void const * argument)
{
  app_bool_t result;
  uint8_t param_size;
  json_item_t item;
  while(1)
  {
  osDelay(REPORT_TASK_INTERVAL);
  /*上报设备状态*/
  //get_ups_status();
  //get_temperature();
  while(1)
  {
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportDeviceStatus\"";
  if(json_body_to_str(&report_device,http_request.param)!=APP_TRUE)
  {
  APP_LOG_ERROR("report param err.\r\n");
  }
  param_size=strlen((const char *)http_request.param);
  http_make_request_size_time_to_str(param_size,2000,http_request.size_time);
  
  result=http_post(&http_request,&http_response,HTTP_RESPONSE_TIMEOUT);
  if(result==APP_TRUE)
  {
  json_set_item_name_value(&item,"code",NULL);
  json_get_item_value_by_name_from_json_str(http_response.json_str,item.name,item.value); 
  /*服务器回应code:"0"*/
  if(strcmp((const char *)item.value,"\"0\"")==0)
  break;
  }
  osDelay(REPORT_TASK_RETRY_TIMEOUT);
  }  
  } 
}
