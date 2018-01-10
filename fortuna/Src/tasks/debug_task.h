#ifndef  __DEBUG_TASK_H__
#define  __DEBUG_TASK_H__

extern osThreadId debug_task_hdl;
void debug_task(void const * argument);

/*调试任务命令长度定义*/

#define  DEBUG_TASK_CMD_EOL_LEN                      2/*调试时字符串结束符号长度*/

#define  DEBUG_TASK_CMD_SET_ADDR_PARAM_LEN           2
#define  DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_PARAM_LEN  1
#define  DEBUG_TASK_CMD_UNLOCK_DEVICE_PARAM_LEN      1
#define  DEBUG_TASK_CMD_LOCK_DEVICE_PARAM_LEN        1
#define  DEBUG_TASK_CMD_SET_MAX_WEIGHT_PARAM_LEN     1
#define  DEBUG_TASK_CMD_SET_DIVISION_PARAM_LEN       1
#define  DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN   6


/*调试任务命令值定义*/
#define  DEBUG_TASK_CMD_SET_ADDR               "设置地址"    
#define  DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT      "获取净重" 
#define  DEBUG_TASK_CMD_UNLOCK_DEVICE          "解锁设备"
#define  DEBUG_TASK_CMD_LOCK_DEVICE            "上锁设备" 
#define  DEBUG_TASK_CMD_SET_MAX_WEIGHT         "设置最大值" 
#define  DEBUG_TASK_CMD_SET_DIVISION           "设置分度值"
#define  DEBUG_TASK_CALIBRATE_WEIGHT           "校准重量"

/*调试任务超时值定义*/
#define  DEBUG_TASK_WAIT_TIMEOUT                   100
#define  DEBUG_TASK_CMD_UNLOCK_TIMEOUT             1000
#define  DEBUG_TASK_CMD_LOCK_TIMEOUT               1000
#define  DEBUG_TASK_CMD_SET_ADDR_TIMEOUT           1000
#define  DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_TIMEOUT  1000
#define  DEBUG_TASK_CMD_SET_MAX_WEIGHT_TIMEOUT     1000
#define  DEBUG_TASK_CMD_SET_DIVISION_TIMEOUT       1000
#define  DEBUG_TASK_CMD_CALIBRATE_WEIGHT_TIMEOUT   1000

/*调试任务信号*/
#define  DEBUG_TASK_SET_ADDR_OK_SIGNAL          (1<<0)
#define  DEBUG_TASK_SET_ADDR_ERR_SIGNAL         (1<<1)
#define  DEBUG_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL (1<<2)
#define  DEBUG_TASK_OBTAIN_NET_WEIGHT_ERR_SIGNA (1<<3)
#define  DEBUG_TASK_UNLOCK_DEVICE_OK_SIGNAL     (1<<4)
#define  DEBUG_TASK_UNLOCK_DEVICE_ERR_SIGNAL    (1<<5)
#define  DEBUG_TASK_LOCK_DEVICE_OK_SIGNAL       (1<<6)
#define  DEBUG_TASK_LOCK_DEVICE_ERR_SIGNAL      (1<<7)
#define  DEBUG_TASK_SET_MAX_WEIGHT_OK_SIGNAL    (1<<8)
#define  DEBUG_TASK_SET_MAX_WEIGHT_ERR_SIGNAL   (1<<9)
#define  DEBUG_TASK_SET_DIVISION_OK_SIGNAL      (1<<10)
#define  DEBUG_TASK_SET_DIVISION_ERR_SIGNAL     (1<<11)
#define  DEBUG_TASK_CALIBRATE_WEIGHT_OK_SIGNAL  (1<<12)
#define  DEBUG_TASK_CALIBRATE_WEIGHT_ERR_SIGNAL (1<<13)
#define  DEBUG_TASK_ALL_SIGNALS                 ((1<<14)-1)
#endif