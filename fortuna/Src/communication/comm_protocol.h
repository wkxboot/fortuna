#ifndef  __COMM_PROTOCOL__H__
#define  __COMM_PROTOCOL__H__
#include "fortuna_common.h"
#include "scales.h"

typedef enum
{
COMM_OK =0,
COMM_ERR
}comm_status_t;

typedef comm_status_t (*ptr_comm_cmd_process)(uint8_t *ptr_buff,uint8_t param_len,uint8_t *ptr_sen_len);

typedef struct
{
 ptr_comm_cmd_process comm_cmd_process;
 uint8_t cmd_code;
 uint8_t pdu_len;
}comm_cmd_t;

extern osThreadId comm_task_hdl;
/*解析数据帧*/
comm_status_t comm_protocol_parse(uint8_t *ptr_buff,uint8_t recv_len,uint8_t *ptr_send_len);
/*通信初始化*/
comm_status_t comm_init();
/*通信协议接收处理*/
void comm_byte_receive(void);
/*通信协议发送处理*/
void comm_byte_send(void);

/*获取一帧数据的地址和长度*/
comm_status_t comm_receive_fsm(uint8_t **ptr_buff,uint8_t *ptr_recv_len);
/*发送一帧数据*/
comm_status_t comm_send_fsm(uint8_t *ptr_buff,uint8_t send_len);


/*串口通信定义*/
#define  COMM_ADDR                                        1
/*与主机通信串口缓存*/
#define  BUFF_SIZE_MAX                                    64


/*通信协议部分*/
#define  COMM_SERIAL_PDU_SIZE_MIN                          2
#define  COMM_PDU_SIZE_MIN                                 1

/*通信任务子函数信号*/
#define  COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL       (1<<5)
#define  COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_ERR_SIGNAL      (1<<6)
#define  COMM_TASK_CLEAR_ZERO_OK_SIGNAL                    (1<<7)
#define  COMM_TASK_CLEAR_ZERO_ERR_SIGNAL                   (1<<8)
#define  COMM_TASK_CALIBRATE_SCALE_CODE_OK_SIGNAL          (1<<9)
#define  COMM_TASK_CALIBRATE_SCALE_CODE_ERR_SIGNAL         (1<<10)
#define  COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_OK_SIGNAL   (1<<11)
#define  COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_ERR_SIGNAL  (1<<12)
#define  COMM_TASK_CALIBRATE_SCALE_WEIGHT_OK_SIGNAL        (1<<13)
#define  COMM_TASK_CALIBRATE_SCALE_WEIGHT_ERR_SIGNAL       (1<<14)
#define  COMM_TASK_UNLOCK_LOCK_OK_SIGNAL                   (1<<15)
#define  COMM_TASK_UNLOCK_LOCK_ERR_SIGNAL                  (1<<16)
#define  COMM_TASK_LOCK_LOCK_OK_SIGNAL                     (1<<17)
#define  COMM_TASK_LOCK_LOCK_ERR_SIGNAL                    (1<<18)


/*通信任务子函数超时时间*/
#define  COMM_TASK_CALIBRATE_SCALE_CODE_TIMEOUT             500
#define  COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_TIMEOUT      500
#define  COMM_TASK_CALIBRATE_SCALE_WEIGHT_TIMEOUT           500
#define  COMM_TASK_CLEAR_ZERO_TIMEOUT                       400
#define  COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_TIMEOUT          400
#define  COMM_TASK_UNLOCK_LOCK_TIMEOUT                      500
#define  COMM_TASK_LOCK_LOCK_TIMEOUT                        500

/*通信协议部分*/
#define  COMM_CMD_CNT                                       11


#define  COMM_ADDR_OFFSET                                   0
#define  COMM_ADDR_SIZE                                     1
#define  COMM_CMD_OFFSET                                    1
#define  COMM_CMD_SIZE                                      1
#define  COMM_PARAM_OFFSET                                  2

/*目前实际只有4个称重单元*/
#define  COMM_CMD_PARAM_SCALE_MAX                       SCALES_CNT_MAX
/*最大可接入的称重单元数量*/
#define  COMM_VIRTUAL_SCALE_MAX                         20
/*参数部分的长度大小*/
#define  COMM_CMD01_PARAM_SIZE                          1
#define  COMM_CMD02_PARAM_SIZE                          3
#define  COMM_CMD03_PARAM_SIZE                          1
#define  COMM_CMD04_PARAM_SIZE                          0
#define  COMM_CMD11_PARAM_SIZE                          0
#define  COMM_CMD21_PARAM_SIZE                          0
#define  COMM_CMD22_PARAM_SIZE                          0
#define  COMM_CMD23_PARAM_SIZE                          0
#define  COMM_CMD31_PARAM_SIZE                          0
#define  COMM_CMD41_PARAM_SIZE                          0
#define  COMM_CMD51_PARAM_SIZE                          0
/*固定的执行结果值*/
#define  COMM_CMD01_EXECUTE_RESULT_SUCCESS              1 
#define  COMM_CMD01_EXECUTE_RESULT_FAIL                 0
#define  COMM_CMD02_EXECUTE_RESULT_SUCCESS              1 
#define  COMM_CMD02_EXECUTE_RESULT_FAIL                 0
#define  COMM_CMD21_EXECUTE_RESULT_SUCCESS              1
#define  COMM_CMD21_EXECUTE_RESULT_FAIL                 0
#define  COMM_CMD22_EXECUTE_RESULT_SUCCESS              1
#define  COMM_CMD22_EXECUTE_RESULT_FAIL                 0
/*执行结果的长度大小*/
#define  COMM_CMD01_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD02_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD03_EXECUTE_RESULT_ALL_SCALES_SIZE      40
#define  COMM_CMD03_EXECUTE_RESULT_ONE_SCALE_SIZE       2
#define  COMM_CMD04_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD11_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD21_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD22_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD23_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD31_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD41_EXECUTE_RESULT_SIZE                 1
#define  COMM_CMD51_EXECUTE_RESULT_SIZE                 2



#endif