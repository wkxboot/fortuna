#ifndef  __WEIGHT_TASK_H__
#define  __WEIGHT_TASK_H__

/*重量任务handle*/
extern osThreadId weight_task_hdl;






#define  WEIGHT_TASK_OBTAIN_WEIGHT_OK_SIGNAL                 (1<<0)
#define  WEIGHT_TASK_OBTAIN_WEIGHT_ERR_SIGNAL                (1<<1)
#define  WEIGHT_TASK_OBTAIN_FIRMWARE_VERSION_OK_SIGNAL       (1<<2)
#define  WEIGHT_TASK_OBTAIN_FIRMWARE_VERSION_ERR_SIGNAL      (1<<3)
#define  WEIGHT_TASK_SET_MAX_WEIGHT_OK_SIGNAL                (1<<4)
#define  WEIGHT_TASK_SET_MAX_WEIGHT_ERR_SIGNAL               (1<<5)
#define  WEIGHT_TASK_SET_DIVISION_OK_SIGNAL                  (1<<6)
#define  WEIGHT_TASK_SET_DIVISION_ERR_SIGNAL                 (1<<7)
#define  WEIGHT_TASK_RESET_OK_SIGNAL                         (1<<8)
#define  WEIGHT_TASK_RESET_ERR_SIGNAL                        (1<<9)
#define  WEIGHT_TASK_SET_ADDR_OK_SIGNAL                      (1<<10)
#define  WEIGHT_TASK_SET_ADDR_ERR_SIGNAL                     (1<<11)
#define  WEIGHT_TASK_SET_BAUDRATE_OK_SIGNAL                  (1<<12)
#define  WEIGHT_TASK_SET_BAUDRATE_ERR_SIGNAL                 (1<<13)
#define  WEIGHT_TASK_SET_FSM_FORMAT_OK_SIGNAL                (1<<14)
#define  WEIGHT_TASK_SET_FSM_FORMAT_ERR_SIGNAL               (1<<15)
#define  WEIGHT_TASK_SET_PROTOCOL_FORMAT_OK_SIGNAL           (1<<16)
#define  WEIGHT_TASK_SET_PROTOCOL_FORMAT_ERR_SIGNAL          (1<<17)

#define  WEIGHT_TASK_ALL_SIGNALS                             ((1<<18)-1)







#endif