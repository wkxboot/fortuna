#ifndef  __LOCK_TASK_H__
#define  __LOCK_TASK_H__
#include "fortuna_common.h"


#define  LOCK_TASK_INTERVAL                    50/*锁和门的状态更新间隔*/
#define  LOCK_TASK_LOCK_TIMEOUT                500/*开锁的时间 只开一次 jd要求*/

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

uint8_t get_lock_state();
uint8_t get_door_state();

/*锁任务*/
void lock_task(void const * argument);
extern osThreadId lock_task_hdl;
extern osMessageQId lock_task_msg_q_id;

/*锁任务消息*/
#define  LOCK_TASK_LOCK_LOCK_MSG                1
#define  LOCK_TASK_UNLOCK_LOCK_MSG              2

/*协议约定锁的状态值*/
#define  LOCK_TASK_STATE_LOCKED                 0
#define  LOCK_TASK_STATE_UNLOCKED               1
#define  LOCK_TASK_STATE_ERR                    0xff


#endif