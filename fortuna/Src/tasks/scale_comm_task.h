#ifndef  __SCALE_COMM_TASK_H__
#define  __SCALE_COMM_TASK_H__


extern osThreadId scale_comm_task_hdl;
void scale_comm_task(void const * argument);


/*电子秤通信任务同步事件*/
#define  SCALE_COMM_TASK_SYNC_EVT                        (1<<2)





#endif