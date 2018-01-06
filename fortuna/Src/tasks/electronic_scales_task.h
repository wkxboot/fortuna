#ifndef  __ELECTRONIC_SCALES_TASK_H__
#define  __ELECTRONIC_SCALES_TASK_H__

typedef struct
{
 uint8_t type;
 uint8_t scale;
 union
 {
 uint8_t  param8[2];
 uint16_t param16;
 };
}scale_msg_t;

extern osThreadId elec_scales_task_hdl;

#define  ELEC_SCALE_TASK_WAIT_TIMEOUT                      50
/*电子秤任务消息*/
#define  ELEC_SCALE_TASK_SET_TARE_WEIGHT_MSG               1
#define  ELEC_SCALE_TASK_CLEAR_TARE_WEIGHT_MSG             2
#define  ELEC_SCALE_TASK_CALIBRATE_WEIGHT_MSG              3
#define  ELEC_SCALE_TASK_OBTAIN_WEIGHT_MSG                 4
#define  ELEC_SCALE_TASK_OBTAIN_FIRMWARE_VERSION_MSG       5
#define  ELEC_SCALE_TASK_SET_MAX_WEIGHT_MSG                6
#define  ELEC_SCALE_TASK_SET_DIVISION_MSG                  7
#define  ELEC_SCALE_TASK_LOCK_MSG                          8
#define  ELEC_SCALE_TASK_RESET_MSG                         9
#define  ELEC_SCALE_TASK_SET_ADDR_MSG                      10
#define  ELEC_SCALE_TASK_SET_BAUDRATE_MSG                  11
#define  ELEC_SCALE_TASK_SET_FSM_FORMAT_MSG                12
#define  ELEC_SCALE_TASK_SET_PROTOCOL_FORMAT_MSG           13

/*电子秤操作值*/
#define  ELEC_SCALE_TASK_AUTO_TARE_WEIGHT_VALUE            0x7fffffff
#define  ELEC_SCALE_TASK_RESET_VALUE                       0x55







#endif