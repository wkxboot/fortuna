#ifndef  __ELECTRONIC_SCALES_TASK_H__
#define  __ELECTRONIC_SCALES_TASK_H__

typedef struct
{
 uint8_t type;
 uint8_t scale;
 union __param
 {
 uint8_t  param8[2];
 uint16_t param16;
 };
}scale_msg_t;

extern osThreadId elec_scales_task_hdl;




/*电子秤任务消息*/

#define  COMM_CLEAR_SCALE_TARE_WEIGHT_MSG     1
#define  COMM_CALIBRATE_SCALE_WEIGHT_MSG      2
#define  COMM_UNLOCK_LOCK_MSG                 3
#define  COMM_LOCK_LOCK_MSG                   4


#endif