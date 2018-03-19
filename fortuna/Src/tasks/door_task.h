#ifndef  __DOOR_TASK_H__
#define  __DOOR_TASK_H__


extern osThreadId door_task_hdl;
void door_task(void const * argument);


#define  DOOR_TASK_INTERVAL                     50/*门状态监测间隔50ms*/


#define  DOOR_TASK_DOOR_STATUS_OPEN             1
#define  DOOR_TASK_DOOR_STATUS_CLOSE            2
#define  DOOR_TASK_DOOR_STATUS_INIT             3




#endif