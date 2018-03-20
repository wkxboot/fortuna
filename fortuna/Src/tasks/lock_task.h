#ifndef  __LOCK_TASK_H__
#define  __LOCK_TASK_H__
#include "app_common.h"


#define  LOCK_TASK_LOCK_SIGNAL                    (1<<0)
#define  LOCK_TASK_UNLOCK_SIGNAL                  (1<<2)
#define  LOCK_TASK_DOOR_STATUS_CLOSE_SIGNAL       (1<<3)
#define  LOCK_TASK_DOOR_STATUS_OPEN_SIGNAL        (1<<4)
#define  LOCK_TASK_DEBUG_LOCK_SIGNAL              (1<<5)
#define  LOCK_TASK_DEBUG_UNLOCK_SIGNAL            (1<<6) 
#define  LOCK_TASK_ALL_SIGNALS                    ((1<<7)-1)

#define  LOCK_TASK_WAIT_TIMEOUT                   osWaitForever
#define  LOCK_TASK_LOCK_TIMER_TIMEOUT             600
#define  LOCK_TASK_UNLOCK_TIMER_TIMEOUT           600
#define  LOCK_TASK_AUTO_LOCK_TIMER_TIMEOUT        (10*1000)/*10秒钟无人开门自动上锁*/

#define  LOCK_TASK_LOCK_TYPE_AUTO                  1/*自动上锁*/
#define  LOCK_TASK_LOCK_TYPE_MAN                   2/*手动上锁*/


/*锁任务*/
void lock_task(void const * argument);
extern osThreadId lock_task_hdl;



#endif