#ifndef  __SCALE_FUNC_TASK_H__
#define  __SCALE_FUNC_TASK_H__

typedef struct
{
 uint8_t type;
 uint8_t scale;
 union
 {
 uint8_t  param8[2];
 uint16_t param16;
 };
}scale_msg_t;

extern osThreadId scale_func_task_hdl;
extern osMessageQId scale_func_msg_q_id;
void scale_func_task(void const * argument);

/*电子秤功能执行超时时间*/
#define  SCALE_FUNC_TASK_WAIT_TIMEOUT                      50
/*电子秤任务消息*/
#define  SCALE_FUNC_TASK_CLEAR_TARE_WEIGHT_MSG             1
#define  SCALE_FUNC_TASK_CLEAR_ZERO_WEIGHT_MSG             2/*清零*/
#define  SCALE_FUNC_TASK_ZERO_RANGE_SET_MSG                3/*手动清零的范围*/
#define  SCALE_FUNC_TASK_CALIBRATE_WEIGHT_MSG              4
#define  SCALE_FUNC_TASK_OBTAIN_NET_WEIGHT_MSG             5
#define  SCALE_FUNC_TASK_OBTAIN_FIRMWARE_VERSION_MSG       6
#define  SCALE_FUNC_TASK_SET_MAX_WEIGHT_MSG                7
#define  SCALE_FUNC_TASK_SET_DIVISION_MSG                  8
#define  SCALE_FUNC_TASK_LOCK_MSG                          9
#define  SCALE_FUNC_TASK_RESET_MSG                         10
#define  SCALE_FUNC_TASK_SET_ADDR_MSG                      11
#define  SCALE_FUNC_TASK_SET_BAUDRATE_MSG                  12
#define  SCALE_FUNC_TASK_SET_FSM_FORMAT_MSG                13
#define  SCALE_FUNC_TASK_SET_PROTOCOL_FORMAT_MSG           14




/*电子秤功能任务同步事件*/
#define  SCALE_FUNC_TASK_SYNC_EVT                          (1<<1)





#endif