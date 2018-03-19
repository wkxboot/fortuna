#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "string.h"
#include "json.h"
#include "http_get_post.h"
#include "shopping_task.h"
#include "lock_task.h"

#define APP_LOG_MODULE_NAME   "[shopping]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


osThreadId shopping_task_hdl;

static http_request_t http_request;
static http_response_t http_response;

json_pull_open_instruction_t pull_open;
json_report_open_status_t    report_open;
json_report_close_status_t   report_close;
json_report_device_status_t  report_device;


static void shopping_task_init()
{
/*拉取开门指令json*/
pull_open.header.item_cnt=2;
json_set_item_name_value(&pull_open.pid,"\"pid\"","\"011201711022810\""); 
json_set_item_name_value(&pull_open.version,"\"version\"","\"12\""); 
/*开门上报json*/
report_open.header.item_cnt=5;
json_set_item_name_value(&report_open.pid,"\"pid\"","\"011201711022810\""); 
json_set_item_name_value(&report_open.version,"\"version\"","\"12\"");  
json_set_item_name_value(&report_open.open,"\"open\"",NULL); 
json_set_item_name_value(&report_open.open_uuid,"\"openUuid\"",NULL); 
json_set_item_name_value(&report_open.error,"\"error\"",NULL);  
/*关门上报json*/
report_close.header.item_cnt=6;
json_set_item_name_value(&report_close.pid,"\"pid\"","\"011201711022810\""); 
json_set_item_name_value(&report_close.version,"\"version\"","\"12\""); 
json_set_item_name_value(&report_close.user_pin,"\"userPin\"","\"011201711022810\""); 
json_set_item_name_value(&report_close.open_uuid,"\"openUuid\"",NULL);  
json_set_item_name_value(&report_close.type,"\"type\"",NULL); 
json_set_item_name_value(&report_close.auto_lock,"\"autoLock\"","0"); 
/*设备状态上报json*/
report_device.header.item_cnt=11;
json_set_item_name_value(&report_device.pid,"\"pid\"","\"011201711022810\""); 
json_set_item_name_value(&report_device.version,"\"version\"","\"12\"");    
json_set_item_name_value(&report_device.ip,"\"ip\"","\"12.34.56.78\"");  
json_set_item_name_value(&report_device.m_power,"\"mPower\"","1"); /*主电源状态*/
json_set_item_name_value(&report_device.e_power,"\"ePower\"","1"); /*备用电源状态*/ 
json_set_item_name_value(&report_device.lock,"\"lock\"","1");  /*锁状态*/ 
json_set_item_name_value(&report_device.net,"\"net\"","12");  
json_set_item_name_value(&report_device.rssi,"\"rssi\"","29"); 
json_set_item_name_value(&report_device.push_id,"\"pushId\"","\"011201711022810\""); 
json_set_item_name_value(&report_device.boot,"\"boot\"","1");  /*启动状态*/ 
json_set_item_name_value(&report_device.temperature,"\"temperature\"","12"); /*温度*/   
}

uint8_t emulate[]="{\"result\":{\"expire\":\"1521080916800\",\"token\":\"e76aebcfa6096a70994d69e5811430c3d3836a4a\",\"data\":{\"userPin\":\"JD_20874f3ca9d474f\",\"type\":0},\"uuid\":\"30bbf00e09664194b0d2442a0210dace\"},\"code\":\"0\",\"msg\":\"成功\"}";
/*购物流程任务*/
void shopping_task(void const * argument)
{
 osEvent sig;
 app_bool_t result;
 uint8_t param_size;
 json_item_t item;
 APP_LOG_INFO("######购货任务开始.\r\n");
 
 shopping_task_init();
 
 do
 {
 result=http_init();
 }while(result!=APP_TRUE);

 APP_LOG_DEBUG("购物任务初始化成功.\r\n");
 
 while(1)
 {
  osDelay(SHOPPING_TASK_INTERVAL); 
 
  /*拉取开门指令*/
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/pullOpenInstruction\"";
  if(json_body_to_str(&pull_open,http_request.param)!=APP_TRUE)
  {
  APP_LOG_ERROR("pull open param err.\r\n");
  }
  param_size=strlen((const char *)http_request.param);
  http_make_request_size_time_to_str(param_size,2000,http_request.size_time);
  result=http_post(&http_request,&http_response,HTTP_RESPONSE_TIMEOUT);
  if(result!=APP_TRUE)
  continue;
  
  json_set_item_name_value(&item,"uuid",NULL);
  result=json_get_item_value_by_name_from_json_str(http_response.json_str,item.name,item.value); 
  /*如果没有找到uuid*/
  if(result!=APP_TRUE)
  {
  APP_LOG_DEBUG("没有开门指令.继续请求...\r\n");
  continue;
  }
  
  //strcpy((char *)http_response.json_str,(const char *)emulate);
  APP_LOG_DEBUG("收到开门指令.\r\n");
  
  /*拷贝json中uuid的值到report open中的open_uuid*/
  result=json_get_item_value_by_name_from_json_str(http_response.json_str,"uuid",report_open.open_uuid.value);
  /*拷贝json中uuid的值到report close中的open_uuid*/
  result=json_get_item_value_by_name_from_json_str(http_response.json_str,"uuid",report_close.open_uuid.value);
  /*拷贝json中userpin的值到report close中的userpin*/
  result=json_get_item_value_by_name_from_json_str(http_response.json_str,report_close.user_pin.name,report_close.user_pin.value);
  /*拷贝json中type的值到report close中的type*/
  result=json_get_item_value_by_name_from_json_str(http_response.json_str,report_close.type.name,report_close.type.value);
  
  /*操作开锁*/
  osSignalSet(lock_task_hdl,LOCK_TASK_UNLOCK_SIGNAL);
  /*等待完成操作*/
  sig=osSignalWait(SHOPPING_TASK_ALL_SIGNALS,osWaitForever);
  /*开锁成功*/
  if(sig.status==osEventSignal && sig.value.signals & SHOPPING_TASK_UNLOCK_LOCK_SUCCESS_SIGNAL)
  {
  json_set_item_name_value(&report_open.open,NULL,"true");
  json_set_item_name_value(&report_open.error,NULL,"0");
  APP_LOG_DEBUG("开锁成功.\r\n"); 
  }
  else
  {
  json_set_item_name_value(&report_open.open,NULL,"false");
  json_set_item_name_value(&report_open.error,NULL,"4");
  APP_LOG_DEBUG("开锁失败.\r\n");  
  }
 /*上报开门状态*/
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportLockOpenStatus\"";
  if(json_body_to_str(&report_open,http_request.param)!=APP_TRUE)
  {
  APP_LOG_ERROR("report open param err.\r\n");
  }
  param_size=strlen((const char *)http_request.param);
  http_make_request_size_time_to_str(param_size,2000,http_request.size_time);
  while(1)
  {
  result=http_post(&http_request,&http_response,HTTP_RESPONSE_TIMEOUT);
  if(result==APP_TRUE)
  {
  json_set_item_name_value(&item,"code",NULL);
  json_get_item_value_by_name_from_json_str(http_response.json_str,item.name,item.value); 
  if(strcmp((const char *)item.value,"\"0\"")==0)
  break;
  }
  osDelay(SHOPPING_TASK_RETRY_TIMEOUT);
  } 
  /*服务器回应code:"0"*/
  /*等待关门*/
  while(1)
  {
  sig=osSignalWait(SHOPPING_TASK_ALL_SIGNALS,osWaitForever);
  /*关门成功*/
  if(sig.status==osEventSignal && sig.value.signals & SHOPPING_TASK_LOCK_LOCK_SUCCESS_SIGNAL)
  break;
  }
  
  /*上报关门状态*/  
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportSaleSkuInfo\"";
  if(json_body_to_str(&report_close,http_request.param)!=APP_TRUE)
  {
  APP_LOG_ERROR("report close param err.\r\n");
  }
  param_size=strlen((const char *)http_request.param);
  http_make_request_size_time_to_str(param_size,2000,http_request.size_time);
  while(1)
  {
  result=http_post(&http_request,&http_response,HTTP_RESPONSE_TIMEOUT);
  if(result==APP_TRUE)
  {
  json_set_item_name_value(&item,"code",NULL);
  json_get_item_value_by_name_from_json_str(http_response.json_str,item.name,item.value); 
  if(strcmp((const char *)item.value,"\"0\"")==0)/*服务器回应code:"0"*/ 
  break;
  }
  osDelay(SHOPPING_TASK_RETRY_TIMEOUT);
  }
 }
}
