#ifndef  __SHOPPING_TASK_H__
#define  __SHOPPING_TASK_H__
#include "app_common.h"

/*购物流程任务*/
void shopping_task(void const * argument);
/*主机通信任务任务handle*/
extern osThreadId shopping_task_hdl;

#define  SHOPPING_TASK_INTERVAL                           100/*购物任务运行间隔*/
#define  SHOPPING_PULL_OPEN_TIMEOUT                       1000/*购物任务拉取开门指令时间*/
#define  SHOPPING_TASK_LOCK_CTL_TIMEOUT                   500/*开锁等待时间*/
#define  SHOPPING_TASK_AUTO_LOCK_DETECT_INTERVAL          800/*门检测时间间隔*/
#define  SHOPPING_TASK_AUTO_LOCK_TIMEOUT                 (1000*10)/*门没有被拉开的超时时间*/

#endif