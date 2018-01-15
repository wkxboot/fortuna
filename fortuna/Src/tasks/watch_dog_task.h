#ifndef  __WATCH_DOG_TASK_H__
#define  __WATCH_DOG_TASK_H__

/*看门狗任务*/
void watch_dog_task(void const * argument);
extern osThreadId watch_dog_task_hdl;


#define  WATCH_DOG_TASK_INTERVAL                200/*200ms喂一次看门狗*/





#endif