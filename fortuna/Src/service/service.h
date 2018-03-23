#ifndef  __SERVICE_H__
#define  __SERVICE_H__

#define  IMEI_STR_SIZE                                  15
#define  SERVICE_NET_ERR_CNT_MAX                        5
#define  SERVICE_DEVICE_ERR_CNT_MAX                     5

#define  SERVICE_RESET_HOLD_ON_TIME                     2000
#define  SERVICE_RESET_WAIT_TIME                        15000/*等待15秒*/

#define  HTTP_RESPONSE_TIMEOUT                          40000/*40秒超时*/

#define  SIM800A_MODULE                                 1
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



app_bool_t service_http_post(http_request_t *ptr_request,http_response_t *ptr_response,uint16_t response_timeout);

void service_http_make_request_size_time_to_str(uint16_t size,uint16_t time,uint8_t *ptr_str);
/*获取rssi值*/
app_bool_t service_get_rssi_str(uint8_t *ptr_rssi_str);
/*获取IP地址字符串*/
app_bool_t service_get_ip_str(uint8_t *ptr_ip_str);
/*获取IMEI字符串*/
app_bool_t service_get_imei_str(uint8_t *ptr_imei_str);

app_bool_t service_reset();


#endif