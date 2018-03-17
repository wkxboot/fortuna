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

osMutexId device_mutex_id;

static void device_mutex_init()
{
   /*创建互斥体*/
 osMutexDef(device_mutex);
 device_mutex_id=osMutexCreate(osMutex(device_mutex)); 
 APP_ASSERT(device_mutex_id);
}

/*获取互斥体*/
static void take_device_mutex()
{
 if(osMutexWait(device_mutex_id,osWaitForever)!=osOK)
 {
  APP_ERROR_HANDLER(0);
 }
}

/*释放互斥体*/
static void release_device_mutex()
{
 osMutexRelease(device_mutex_id);
}




http_request_t http_request;
http_response_t http_response;
json_pull_open_instruction_t pull_open;
json_report_open_status_t    report_open;
json_report_close_status_t   report_close;
json_report_device_status_t  report_device;

#define  BSP_LOCK_CTL_UNLOCK     1
#define  BSP_LOCK_CTL_LOCK     2
#define  BSP_LOCK_STATUS_UNLOCK     1
#define  BSP_LOCK_STATUS_LOCK     2

#define  BSP_DOOR_STATUS_CLOSE     1
#define  BSP_DOOR_STATUS_OPEN    2

void bsp_lock_ctl(uint8_t ctl)
{
}
uint8_t bsp_get_lock_status()
{
return BSP_LOCK_CTL_UNLOCK;
}
uint8_t bsp_get_door_status()
{
return BSP_DOOR_STATUS_CLOSE;
}





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
json_set_item_name_value(&report_close.auto_lock,"\"autoLock\"",NULL); 
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
json_set_item_name_value(&report_device.push_id,"\"pushId\"",NULL); 
json_set_item_name_value(&report_device.boot,"\"boot\"","1");  /*启动状态*/ 
json_set_item_name_value(&report_device.temperature,"\"temperature\"","12"); /*温度*/   
}

uint8_t emulate[]="{\"result\":{\"expire\":\"1521080916800\",\"token\":\"e76aebcfa6096a70994d69e5811430c3d3836a4a\",\"data\":{\"userPin\":\"JD_20874f3ca9d474f\",\"type\":0},\"uuid\":\"30bbf00e09664194b0d2442a0210dace\"},\"code\":\"0\",\"msg\":\"成功\"}";
/*购物流程任务*/
void shopping_task(void const * argument)
{
 app_bool_t result;
 app_bool_t is_unlock_success;
 app_bool_t is_door_in_auto_lock_status;
 uint8_t param_size;
 json_item_t item;
 uint16_t pull_open_time=0;
 uint16_t auto_lock_timeout=0;
 
 
 APP_LOG_INFO("######购货任务开始.\r\n");
 device_mutex_init();
 shopping_task_init();
 
 take_device_mutex();
 do
 {
 result=http_init();
 }while(result!=APP_TRUE);
 release_device_mutex();

 APP_LOG_DEBUG("购物任务初始化成功.\r\n");
 
 while(1)
 {
 release_device_mutex();
 osDelay(SHOPPING_TASK_INTERVAL); 
 
 take_device_mutex();
 
 pull_open_time+=SHOPPING_TASK_INTERVAL;
 if(pull_open_time>=SHOPPING_PULL_OPEN_TIMEOUT)
 {
  pull_open_time=0;
  /*拉取开门指令*/
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/pullOpenInstruction\"";
  if(json_body_to_str(&pull_open,&http_request.ptr_param)!=APP_TRUE)
  {
  APP_LOG_ERROR("pull open param err.\r\n");
  }
  param_size=strlen((const char *)http_request.ptr_param);
  http_make_request_size_time_to_str(param_size,2000,http_request.size_time);
  result=http_post(&http_request,&http_response,100);
  if(result!=APP_TRUE)
  continue;
  /*
  if(result!=APP_TRUE)
  {
  APP_LOG_DEBUG("没有开门指令.继续请求...\r\n");
  continue;
  }
  */
  http_response.ptr_json=emulate;
  APP_LOG_DEBUG("收到开门指令.\r\n");
  
  /*拷贝json中uuid的值到report open中的open_uuid*/
  result=json_get_value_by_name_from_json_body(http_response.ptr_json,"uuid",report_open.open_uuid.value);
  /*拷贝json中uuid的值到report device的push_uuid*/
  result=json_get_value_by_name_from_json_body(http_response.ptr_json,"uuid",report_device.push_id.value);
  /*拷贝json中uuid的值到report close中的open_uuid*/
  result=json_get_value_by_name_from_json_body(http_response.ptr_json,"uuid",report_close.open_uuid.value);
  /*拷贝json中userpin的值到report close中的userpin*/
  result=json_get_value_by_name_from_json_body(http_response.ptr_json,report_close.user_pin.name,report_close.user_pin.value);
  /*拷贝json中type的值到report close中的type*/
  result=json_get_value_by_name_from_json_body(http_response.ptr_json,report_close.type.name,report_close.type.value);
  /*操作开锁*/
  bsp_lock_ctl(BSP_LOCK_CTL_UNLOCK);
  /*等待完成操作*/
  osDelay(SHOPPING_TASK_LOCK_CTL_TIMEOUT);
  /*如果锁打开*/
  if(bsp_get_lock_status()==BSP_LOCK_STATUS_UNLOCK)
  {
  is_unlock_success=APP_TRUE;
  json_set_item_name_value(&report_open.open,NULL,"true");
  json_set_item_name_value(&report_open.error,NULL,"0");
  APP_LOG_DEBUG("开锁成功.\r\n");
  }
  else
  {
  is_unlock_success=APP_FALSE;
  json_set_item_name_value(&report_open.open,NULL,"false");
  json_set_item_name_value(&report_open.error,NULL,"4");
  /*操作关锁*/
  bsp_lock_ctl(BSP_LOCK_CTL_LOCK);
  APP_LOG_DEBUG("开锁失败.\r\n");  
  }
 /*上报开门状态*/
  take_device_mutex();
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportLockOpenStatus\"";
  if(json_body_to_str(&report_open,&http_request.ptr_param)!=APP_TRUE)
  {
  APP_LOG_ERROR("report open param err.\r\n");
  }
  param_size=strlen((const char *)http_request.ptr_param);
  http_make_request_size_time_to_str(param_size,1000,http_request.size_time);
  while(1)
  {
  result=http_post(&http_request,&http_response,100);
  if(result==APP_TRUE)
  {
  json_set_item_name_value(&item,"code",NULL);
  json_get_value_by_name_from_json_body(http_response.ptr_json,item.name,item.value); 
  if(result==APP_TRUE && strcmp((const char *)item.value,"\"0\"")==0)
  break;
  }
  }  /*服务器回应code:"0"*/
  release_device_mutex();
  if(is_unlock_success!=APP_TRUE)
  continue;
  /*等待关门*/
  auto_lock_timeout=0;
  is_door_in_auto_lock_status=APP_TRUE;
  while(1)
  {
  osDelay(SHOPPING_TASK_AUTO_LOCK_DETECT_INTERVAL);
  if(bsp_get_door_status()==BSP_DOOR_STATUS_CLOSE)/*如果是关闭的*/
  {
  if(is_door_in_auto_lock_status==APP_TRUE)
  {
   auto_lock_timeout+=SHOPPING_TASK_INTERVAL;
   if(auto_lock_timeout>=SHOPPING_TASK_AUTO_LOCK_TIMEOUT)
   {
   /*操作关锁*/
   bsp_lock_ctl(BSP_LOCK_CTL_LOCK);
   /*等待完成操作*/
   osDelay(SHOPPING_TASK_LOCK_CTL_TIMEOUT); 
   if(bsp_get_lock_status()==BSP_LOCK_STATUS_LOCK)/*如果是锁上了*/
   {
    json_set_item_name_value(&report_close.auto_lock,NULL,"0");/*自动上锁标志*/
    goto report_clsoe_handle;
   }
   else
   {
   is_door_in_auto_lock_status=APP_FALSE;
   }
   } 
  }
  else
  {
   /*操作关锁*/
   bsp_lock_ctl(BSP_LOCK_CTL_LOCK);
   /*等待完成操作*/
   osDelay(SHOPPING_TASK_LOCK_CTL_TIMEOUT); 
   if(bsp_get_lock_status()==BSP_LOCK_STATUS_LOCK)/*如果是锁上了*/
   {
    json_set_item_name_value(&report_close.auto_lock,NULL,"1");/*手动上锁标志*/
    goto report_clsoe_handle;
   }
   else
   {
   /*操作开锁*/
   bsp_lock_ctl(BSP_LOCK_CTL_UNLOCK); 
   }
  }
  }
  else
  {
  is_door_in_auto_lock_status=APP_FALSE; 
  }
  }
  /*上报关门状态*/  
report_clsoe_handle:
  http_request.ptr_url="\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportSaleSkuInfo\"";
  if(json_body_to_str(&report_close,&http_request.ptr_param)!=APP_TRUE)
  {
  APP_LOG_ERROR("report close param err.\r\n");
  }
  param_size=strlen((const char *)http_request.ptr_param);
  http_make_request_size_time_to_str(param_size,100,http_request.size_time);
  do
  {
  result=http_post(&http_request,&http_response,100);
  json_set_item_name_value(&item,"code",NULL);
  json_get_value_by_name_from_json_body(http_response.ptr_json,item.name,item.value); 
  }
  while(result!=APP_TRUE || strcmp((const char *)item.value,"0")!=0);  /*服务器回应code:"0"*/ 
  }
 }
}
