#ifndef  __HOST_COMM_TASK_H__
#define  __HOST_COMM_TASK_H__
#include "fortuna_common.h"

/*主机通信任务*/
void host_comm_task(void const * argument);
/*主机通信任务任务handle*/
extern osThreadId host_comm_task_hdl;


/*串口通信定义*/
#define  COMM_ADDR                                   1
#define  COMM_PORT                                   0
#define  COMM_BAUDRATE                               9600UL
#define  COMM_DATABITS                               8

/*主机通信任务信号*/
#define  HOST_COMM_TASK_BUFF_OVERFLOW_SIGNAL               (1<<0)
#define  HOST_COMM_TASK_RECV_FSM_SIGNAL                    (1<<1)
#define  HOST_COMM_TASK_PARSE_PROTOCOL_SIGNAL              (1<<2)
#define  HOST_COMM_TASK_SEND_FSM_SIGNAL                    (1<<3)
#define  HOST_COMM_TASK_SEND_FSM_OVER_SIGNAL               (1<<4)

#define  HOST_COMM_TASK_ALL_SIGNALS                        ((1<<13)-1)

/*主机通信任务同步事件*/
#define  HOST_COMM_TASK_SYNC_EVT                           (1<<3)








#endif