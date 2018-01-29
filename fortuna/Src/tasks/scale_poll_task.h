#ifndef  __SCALE_POLL_TASK_H__
#define  __SCALE_POLL_TASK_H__

/*重量任务handle*/
extern osMessageQId scale_poll_msg_q_id;
extern osThreadId scale_poll_task_hdl;
void scale_poll_task(void const * argument);

/*电子秤净重轮询任务运行间隔*/
#define  SCALE_POLL_TASK_INTERVAL                                10



/*电子秤轮询任务信号*/
#define  SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL             (1<<0)
#define  SCALE_POLL_TASK_OBTAIN_NET_WEIGHT_ERR_SIGNAL            (1<<1)
#define  SCALE_POLL_TASK_OBTAIN_FIRMWARE_VERSION_OK_SIGNAL       (1<<2)
#define  SCALE_POLL_TASK_OBTAIN_FIRMWARE_VERSION_ERR_SIGNAL      (1<<3)
#define  SCALE_POLL_TASK_SET_MAX_WEIGHT_OK_SIGNAL                (1<<4)
#define  SCALE_POLL_TASK_SET_MAX_WEIGHT_ERR_SIGNAL               (1<<5)
#define  SCALE_POLL_TASK_SET_DIVISION_OK_SIGNAL                  (1<<6)
#define  SCALE_POLL_TASK_SET_DIVISION_ERR_SIGNAL                 (1<<7)
#define  SCALE_POLL_TASK_RESET_OK_SIGNAL                         (1<<8)
#define  SCALE_POLL_TASK_RESET_ERR_SIGNAL                        (1<<9)
#define  SCALE_POLL_TASK_SET_ADDR_OK_SIGNAL                      (1<<10)
#define  SCALE_POLL_TASK_SET_ADDR_ERR_SIGNAL                     (1<<11)
#define  SCALE_POLL_TASK_SET_BAUDRATE_OK_SIGNAL                  (1<<12)
#define  SCALE_POLL_TASK_SET_BAUDRATE_ERR_SIGNAL                 (1<<13)
#define  SCALE_POLL_TASK_SET_FSM_FORMAT_OK_SIGNAL                (1<<14)
#define  SCALE_POLL_TASK_SET_FSM_FORMAT_ERR_SIGNAL               (1<<15)
#define  SCALE_POLL_TASK_SET_PROTOCOL_FORMAT_OK_SIGNAL           (1<<16)
#define  SCALE_POLL_TASK_SET_PROTOCOL_FORMAT_ERR_SIGNAL          (1<<17)
#define  SCALE_POLL_TASK_LOCK_OK_SIGNAL                          (1<<18)
#define  SCALE_POLL_TASK_LOCK_ERR_SIGNAL                         (1<<19)
#define  SCALE_POLL_TASK_ALL_SIGNALS                             ((1<<20)-1)

/*电子秤轮询任务同步事件*/
#define  SCALE_POLL_TASK_SYNC_EVT                                (1<<0)




#endif