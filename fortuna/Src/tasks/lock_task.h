#ifndef  __LOCK_TASK_H__
#define  __LOCK_TASK_H__
#include "fortuna_common.h"

typedef struct
{
 uint8_t type;
 uint8_t lock;
 union 
 {
 uint8_t  param8[2];
 uint16_t param16;
 };
}lock_msg_t;

extern uint8_t lock_status;
extern uint8_t door_status;

/*锁任务handle*/
extern osThreadId lock_task_hdl;
extern osMessageQId lock_task_msg_q_id;

/*锁任务消息*/
#define  LOCK_TASK_LOCK_LOCK_MSG                1
#define  LOCK_TASK_UNLOCK_LOCK_MSG              2



#endif