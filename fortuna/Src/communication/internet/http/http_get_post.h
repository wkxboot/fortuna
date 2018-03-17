#ifndef  __HTTP_GET_POST_H__
#define  __HTTP_GET_POST_H__

#define  HTTP_CONFIG_AT_CMD_RESPONSE_TIMEOUT    100
#define  HTTP_CONFIG_RESPONSE_DELAY_TIMEOUT     10000

#define  AT_MODULE_SIOT203                      1
#define  SIM900A_MODULE                         2
#define  AT_MODULE                              SIM900A_MODULE



typedef struct
{
  uint8_t *ptr_url;
  uint8_t *ptr_param;
  uint8_t size_time[10];
}http_request_t;

typedef struct
{
 uint8_t *ptr_json;
}http_response_t;



app_bool_t http_init();
app_bool_t http_post(http_request_t *ptr_request,http_response_t *ptr_response,uint16_t response_timeout);

void http_make_request_size_time_to_str(uint16_t size,uint16_t time,uint8_t *ptr_str);





#endif