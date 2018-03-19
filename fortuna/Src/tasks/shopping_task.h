#ifndef  __SHOPPING_TASK_H__
#define  __SHOPPING_TASK_H__
#include "app_common.h"

/*购物流程任务*/
void shopping_task(void const * argument);
/*主机通信任务任务handle*/
extern osThreadId shopping_task_hdl;

#define  SHOPPING_TASK_INTERVAL                           100/*购物任务运行间隔*/
#define  SHOPPING_TASK_RETRY_TIMEOUT                      100

#define  SHOPPING_TASK_LOCK_LOCK_SUCCESS_SIGNAL           (1<<0)
#define  SHOPPING_TASK_LOCK_LOCK_FAIL_SIGNAL              (1<<1)
#define  SHOPPING_TASK_UNLOCK_LOCK_SUCCESS_SIGNAL         (1<<2)
#define  SHOPPING_TASK_UNLOCK_LOCK_FAIL_SIGNAL            (1<<3)

#define  SHOPPING_TASK_ALL_SIGNALS                        ((1<<4)-1)


#endif