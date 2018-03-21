#ifndef  __HTTP_H__
#define  __HTTP_H__

#define  HTTP_CONFIG_AT_CMD_NORMAL_RESPONSE_TIMEOUT     100
#define  HTTP_CONFIG_AT_CMD_SPECIAL_RESPONSE_TIMEOUT    5000
#define  HTTP_RESPONSE_TIMEOUT                          40000/*40秒超时*/

#define  AT_MODULE_SIOT203                              1
#define  SIM900A_MODULE                                 2
#define  AT_MODULE                                      SIM900A_MODULE


#define  HTTP_REQUEST_STR_MAX_SIZE                      250
#define  HTTP_REQUEST_SIZE_TIME_STR_MAX_SIZE            12
#define  HTTP_RESPONSE_STR_MAX_SIZE                     250

typedef struct
{
  uint8_t *ptr_url;
  uint8_t param[HTTP_REQUEST_STR_MAX_SIZE];
  uint8_t size_time[HTTP_REQUEST_SIZE_TIME_STR_MAX_SIZE];
}http_request_t;

typedef struct
{
 uint8_t size;
 uint8_t json_str[HTTP_RESPONSE_STR_MAX_SIZE];
}http_response_t;


typedef struct
{
 uint8_t ip[20];
 uint8_t rssi[5];
}http_monitor_response;


app_bool_t http_init();
app_bool_t http_post(http_request_t *ptr_request,http_response_t *ptr_response,uint16_t response_timeout);

void http_make_request_size_time_to_str(uint16_t size,uint16_t time,uint8_t *ptr_str);
/*http模块状态监视*/
app_bool_t http_device_status_monitor(http_monitor_response *ptr_monitor);



#endif