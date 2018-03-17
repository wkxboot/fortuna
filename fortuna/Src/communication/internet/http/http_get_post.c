#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "at_cmd_set.h"
#include "json.h"
#include "http_get_post.h"
#define APP_LOG_MODULE_NAME   "[httpd]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"
#include "stdlib.h"



at_cmd_response_t at_cmd_response;



#if (AT_MODULE == SIM900A_MODULE )  



app_bool_t http_init()
{
app_bool_t result=APP_TRUE;
at_cmd_status_t status;

at_cmd_init();

 /*需要等待5000ms*/
 osDelay(5000);
 /*进入AT测试模式*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
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
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
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
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
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
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+SAPBR","1,1",&at_cmd_response);
 
 /*
if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBRP打开参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 { 
  APP_LOG_ERROR("++SAPBR打开参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
*/
 /*第四步 http初始化AT+HTTPINIT*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+HTTPINIT",&at_cmd_response);
 /*
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPINIT参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPINIT参数设置失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
*/
  /*第五步设置内容类型AT+HTTPPARA="CONTENT","application/json"*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
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
  ptr_response->ptr_json=NULL;
  /*第一步设置url AT+HTTPPARA="URL","http://xxxx"*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
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
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
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
  goto err_handle;  
 }
 APP_LOG_DEBUG("+HTTPDATA POST设置成功. status:%d\r\n",status);
 
 /*第三步设置缓存POST数据“......”*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
 status=at_cmd_string(ptr_request->ptr_param,&at_cmd_response);
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
 at_cmd_response.response_delay_timeout=HTTP_CONFIG_RESPONSE_DELAY_TIMEOUT;
 at_cmd_response.response_timeout=HTTP_CONFIG_RESPONSE_DELAY_TIMEOUT;
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
 at_cmd_response.response_timeout=HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+HTTPREAD",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("读POST回应数据输入失败 status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;
 }
 if(json_find_json_body(at_cmd_response.ptr_response,&ptr_response->ptr_json)!=APP_TRUE)
 {
  APP_LOG_ERROR("http回复没有json格式.\r\n");
  result=APP_FALSE;
  goto err_handle;
 }
 APP_LOG_DEBUG("http回复中找到json格式.\r\n");
err_handle:
 return result;
}

static void uint16_to_str(uint16_t num,uint8_t *ptr_str)
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
  uint16_to_str(size,ptr_str);/*写入数据大小字符串3位*/
  ptr_str[4]=',';/*写入','字符串1位*/
  uint16_to_str(time,ptr_str+5);/*写入时间大小字符串3位*/
}

#else

#endif
