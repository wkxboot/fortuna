#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "at_cmd_set.h"
#include "string.h"
#include "json.h"
#include "http.h"
#define APP_LOG_MODULE_NAME   "[httpd]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"
#include "stdlib.h"

static void http_uint16_to_str(uint16_t num,uint8_t *ptr_str);

static at_cmd_response_t at_cmd_response;

osMutexId http_mutex_id;



static void http_mutex_init()
{
   /*创建互斥体*/
  if(http_mutex_id==NULL)
  {
   osMutexDef(http_mutex);
   http_mutex_id=osMutexCreate(osMutex(http_mutex)); 
   APP_ASSERT(http_mutex_id);
  }
}

/*获取互斥体*/
static void take_http_mutex()
{
 
 if(osMutexWait(http_mutex_id,osWaitForever)!=osOK)
 {
  APP_ERROR_HANDLER(0);
 }

}

/*释放互斥体*/
static void release_http_mutex()
{
  
 if(osMutexRelease(http_mutex_id)!=osOK)
 {
  APP_ERROR_HANDLER(0);
 }

}

#if (AT_MODULE == SIM900A_MODULE )  



app_bool_t http_init()
{
 app_bool_t result=APP_TRUE;
 at_cmd_status_t status;
 if(http_mutex_id==NULL)
 http_mutex_init();
 
 at_cmd_init();

 /*需要等待模块启动完毕*/
 osDelay(5000);
 /*进入AT测试模式*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_cmd_string("AT",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("AT命令测试失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("AT命令测试失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("AT命令测试成功. status:%d.\r\n",status); 

 /*第一步 设置数据承载模式AT+SAPBR=3,1,"CONTYPE","GPRS" */
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+SAPBR","3,1,\"CONTYPE\",\"GPRS\"",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBR承载参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBR承载参数设置失败. status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;  
 }
 /*第二步 设置数据运营商AT+SAPBR=3,1,"APN","CMNET" */
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+SAPBR","3,1,\"APN\",\"CMNET\"",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBR运营商参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+SAPBR运营商参数设置失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 
 /*第三步 打开数据承载模式AT+SAPBR=1,1*/ 
 at_cmd_response.is_response_delay=AT_CMD_TRUE;
 at_cmd_response.response_delay_timeout=HTTP_CONFIG_AT_CMD_SPECIAL_RESPONSE_TIMEOUT;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_SPECIAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+SAPBR","1,1",&at_cmd_response);
 
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBRP打开承载模式参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 { 
   /*如果回应没有OK，就默认是已经打开的*/
   if(status==AT_CMD_STATUS_INVALID_RESPONSE)
   {
    APP_LOG_ERROR("++SAPBR打开承载模式参数已经设置成功. status:%d.\r\n",status); 
   } 
 }
 APP_LOG_DEBUG("++SAPBR打开承载模式参数设置成功. status:%d.\r\n",status); 
 /*第四步 http初始化AT+HTTPINIT*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_SPECIAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+HTTPINIT",&at_cmd_response);

 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPINIT参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   /*如果回应没有OK，就默认是已经初始化完成*/
   if(status==AT_CMD_STATUS_INVALID_RESPONSE)
   {
    APP_LOG_ERROR("++HTTPINIT参数已经成功. status:%d.\r\n",status); 
   }
 }
 APP_LOG_DEBUG("++HTTPINIT参数设置成功. status:%d\r\n",status);
  /*第五步设置内容类型AT+HTTPPARA="CONTENT","application/json"*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPPARA","\"CONTENT\",\"application/json\"",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPPARA内容类型参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPPARA内容类型参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
err_handle:
 return result;
}

app_bool_t http_post(http_request_t *ptr_request,http_response_t *ptr_response,uint16_t response_timeout)
{
 at_cmd_status_t status;
 app_bool_t result=APP_TRUE;
 /*只有一个http POST通道所以要避免竞争*/
 take_http_mutex();
  
 /*第一步设置url AT+HTTPPARA="URL","http://xxxx"*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPPARA",ptr_request->ptr_url,&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPPARA参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPPARA参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("POST url设置成功. status:%d\r\n",status);
 
 /*第二步设置POST数据大小和超时时间AT+HTTPDATA=size,timeout*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPDATA",ptr_request->size_time,&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPPARA POST参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"DOWNLOAD");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPPARA POST参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("+HTTPDATA POST设置成功. status:%d\r\n",status);
 
 /*第三步设置缓存POST数据“......”*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_cmd_string(ptr_request->param,&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("POST数据输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("POST数据缓存失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("POST数据缓存成功. status:%d\r\n",status); 
 
 /*第四步启动http POST*/
 at_cmd_response.is_response_delay=AT_CMD_TRUE;
 at_cmd_response.response_delay_timeout=response_timeout;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_SPECIAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPACTION","1",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPPARA POST数据输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPPARA POST数据失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("+HTTPDATA POST数据成功. status:%d\r\n",status); 
 status=at_cmd_find_expect_from_response(&at_cmd_response,"1,200,");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("POST数据到服务器失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("POST数据到服务器成功. status:%d.\r\n",status); 
 /*第五步读取回应的数据*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+HTTPREAD",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("读POST回应数据输入失败 status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;
 }

 if(json_find_cpy_json_str_to((uint8_t *)at_cmd_response.response,ptr_response->json_str)!=APP_TRUE)
 {
  APP_LOG_ERROR("http回复没有json格式.\r\n");
  result=APP_FALSE;
  goto err_handle;
 }
  /*赋值回应*/
 ptr_response->size=at_cmd_response.size;
 APP_LOG_DEBUG("http回复中找到json格式.\r\n");
err_handle:
 release_http_mutex();
 return result;
}

static void http_uint16_to_str(uint16_t num,uint8_t *ptr_str)
{
 uint8_t temp;
 uint8_t i;
 uint8_t str[]={'0','1','2','3','4','5','6','7','8','9'};
 if(num>9999)
 {
  for(i=0;i<4;i++)
  {
  ptr_str[i]='9';
  }
  ptr_str[i]=0;
  return;
 }
 temp=num/1000;
 *ptr_str++=str[temp];
 num%=1000;
 temp=num/100;
 *ptr_str++=str[temp];
 num%=100;
 temp=num/10;
 *ptr_str++=str[temp];
 num%=10;
 temp=num;
 *ptr_str++=str[temp];
 *ptr_str=0;
}

void http_make_request_size_time_to_str(uint16_t size,uint16_t time,uint8_t *ptr_str)
{
  http_uint16_to_str(size,ptr_str);/*写入数据大小字符串3位*/
  ptr_str[4]=',';/*写入','字符串1位*/
  http_uint16_to_str(time,ptr_str+5);/*写入时间大小字符串3位*/
}





/*http模块状态监视*/
app_bool_t http_device_status_monitor(http_monitor_response *ptr_monitor)
{
 uint8_t *ptr_start_addr;
 app_bool_t result=APP_TRUE;
 at_cmd_status_t status;
 /*只有一个http POST通道所以要避免竞争*/
 take_http_mutex(); 
 
/*第一步查看信号质量AT+CSQ*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+CSQ",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+CSQ 参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+CSQ 读信号质量失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("+CSQ 读信号质量成功. status:%d\r\n",status);
 /*找到csq开始地址*/
 ptr_start_addr=(uint8_t *)strstr((const char *)at_cmd_response.response,"+CSQ: ");
 if(ptr_start_addr==NULL)
 {
 APP_LOG_ERROR("没有找到+CSQ.\r\n");
 result=APP_FALSE;
 goto err_handle; 
 }
  ptr_start_addr+=6;
  ptr_monitor->rssi[0]=ptr_start_addr[0];
  ptr_monitor->rssi[1]=ptr_start_addr[1];
  ptr_monitor->rssi[2]=0;
  APP_LOG_DEBUG("找到RSSI值.已拷贝.RSSI:%s\r\n",ptr_monitor->rssi);
 
err_handle:
 /*只有一个http POST通道所以要避免竞争*/
 release_http_mutex(); 
 return result;
}

#else

#endif
