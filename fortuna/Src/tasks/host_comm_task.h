#ifndef  __HOST_COMM_TASK_H__
#define  __HOST_COMM_TASK_H__
#include "fortuna_common.h"

/*与上位机的通信任务*/
void host_comm_task(void const * argument);
/*通信任务任务handle*/
extern osThreadId host_comm_task_hdl;

/*通信任务信号*/
#define  HOST_COMM_TASK_BUFF_OVERFLOW_SIGNAL               (1<<0)
#define  HOST_COMM_TASK_RECV_FSM_SIGNAL                    (1<<1)
#define  HOST_COMM_TASK_PARSE_PROTOCOL_SIGNAL              (1<<2)
#define  HOST_COMM_TASK_SEND_FSM_SIGNAL                    (1<<3)
#define  HOST_COMM_TASK_SEND_FSM_OVER_SIGNAL               (1<<4)
#define  HOST_COMM_TASK_ALL_SIGNALS                        ((1<<5)-1)










#endif