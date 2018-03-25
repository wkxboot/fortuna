#ifndef  __AT_CMD_SET_H__
#define  __AT_CMD_SET_H__



#define    MAX_ADU_SIZE                              300


#define    AT_CMD_SEND_TIMEOUT                       25
#define    AT_CMD_RESPONSE_MAX_STR_SZIE              MAX_ADU_SIZE

#define    AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT     100
#define    AT_CMD_CONFIG_SPECIAL_RESPONSE_TIMEOUT    5000

typedef enum 
{
  AT_CMD_FALSE=0,
  AT_CMD_TRUE,
}at_cmd_bool_t;

typedef struct
{
  at_cmd_bool_t is_response_delay;
  uint16_t response_timeout;
  uint16_t response_delay_timeout;
volatile uint16_t size;
volatile uint8_t response[AT_CMD_RESPONSE_MAX_STR_SZIE];
}at_cmd_response_t;

typedef enum
{
  AT_CMD_STATUS_SUCCESS=0,
  AT_CMD_STATUS_INVALID_CMD,
  AT_CMD_STATUS_INVALID_PARAM,
  AT_CMD_STATUS_INVALID_RESPONSE,
  AT_CMD_STATUS_SEND_TIMEOUT,
  AT_CMD_STATUS_RECV_TIMEOUT,
}at_cmd_status_t;

void at_cmd_byte_send();
void at_cmd_byte_receive();
void at_cmd_timer_35_expired();

void at_cmd_init();
/*AT基础指令---AT<CMDPARAM>*/
at_cmd_status_t at_base_cmd(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response);
/*AT拓展测试指令---AT+CMD=?*/
at_cmd_status_t at_ex_cmd_test(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response);
/*AT拓展查询指令---AT+CMD?*/
at_cmd_status_t at_ex_cmd_query(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response);
/*AT拓展设置指令---AT+CMD=PARAM1*/
at_cmd_status_t at_ex_cmd_set(uint8_t *ptr_cmd,uint8_t *ptr_param,at_cmd_response_t *ptr_response);
/*AT拓展执行指令---AT+CMD*/
at_cmd_status_t at_ex_cmd_exe(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response);
/*AT自由数据发送 自定义字符串*/
at_cmd_status_t at_cmd_string(uint8_t *ptr_str,at_cmd_response_t *ptr_response);
/*AT回应中查找期望的字符串*/
at_cmd_status_t at_cmd_find_expect_from_response(at_cmd_response_t *ptr_response,uint8_t *ptr_expect);













#endif