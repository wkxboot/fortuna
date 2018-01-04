#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#include "communication_task.h"
#include "electronic_scales_task.h"
#include "lock_task.h"
#define APP_LOG_MODULE_NAME   "[comm]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

extern uint8_t scale_weight[];
extern uint8_t vendor_id,firmware_version;
extern uint8_t elec_scale_cnt;
extern obj_status_t door_status,lock_status,ups_status;
extern int8_t temperature;

uint8_t timer_frame_1ms;
uint8_t comm_addr;
/*解析数据帧*/
static comm_status_t comm_protocol_parse(uint8_t *ptr_buff,uint8_t recv_len,uint8_t *ptr_send_len);
/*命令码0x01 去除皮重 处理函数*/
static comm_status_t comm_cmd01_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len); 
/*命令码0x02 校准 处理函数*/
static comm_status_t comm_cmd02_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len);
/*命令码0x03 查询重量 处理函数*/
static comm_status_t comm_cmd03_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len); 
/*命令码04 查询称重单元数量 处理函数*/
static comm_status_t comm_cmd04_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len); 
/*命令码0x11 查询门的状态 处理函数*/
static comm_status_t comm_cmd11_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len);
/*命令码21 开锁 处理函数*/
static comm_status_t comm_cmd21_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len);
/*命令码22 关锁 处理函数*/
static comm_status_t comm_cmd22_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len); 
/*命令码0x23 查询锁的状态 处理函数*/
static comm_status_t comm_cmd23_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len);
/*命令码0x31 查询UPS的状态 处理函数*/
static comm_status_t comm_cmd31_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len); 
/*命令码0x41 查询温度 处理函数*/
static comm_status_t comm_cmd41_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len);
/*命令码0x51 查询id和版本号 处理函数*/
static comm_status_t comm_cmd51_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len);

comm_cmd_t comm_cmd[COMM_CMD_CNT]=
{
/*通信PDU长度等于串口数据长度减去地址长度。pdu_len=serial_len-addr_size*/
{comm_cmd01_process,0x01,2},
{comm_cmd02_process,0x02,4},
{comm_cmd03_process,0x03,2},
{comm_cmd04_process,0x04,1},
{comm_cmd11_process,0x11,1},
{comm_cmd21_process,0x21,1},
{comm_cmd22_process,0x22,1},
{comm_cmd23_process,0x23,1},
{comm_cmd31_process,0x31,1},
{comm_cmd41_process,0x41,1},
{comm_cmd51_process,0x51,1}
};

void comm_fsm_timer_expired()
{
 /*关闭串口接收，方便后续数据处理*/
 APP_LOG_INFO("串口定时器到期.\r\n")
 APP_LOG_INFO("关闭串口接收.");
 xcomm_port_serial_enable(FORTUNA_FALSE,FORTUNA_FALSE);
 APP_LOG_INFO("接收完一帧数据.向通信任务发送信号.\r\n");
 /*发送接收完成信号*/
 osSignalSet(comm_task_hdl,COMM_RECV_FSM_SIGNAL);
}


static comm_status_t comm_init(uint8_t slave_addr,uint8_t port,uint32_t baudrate,uint8_t databits)
{
/*本机地址*/
 comm_addr=slave_addr;
 comm_status_t status=COMM_OK;
 
if(xcomm_port_serial_init(port,baudrate,databits)!=COMM_OK)
 {
 status=COMM_ERR;
 APP_LOG_ERROR("通信串口初始化错误！\r\n");
 }
 if(baudrate>19200)
 {
   timer_frame_1ms=2;/*波特率超过19200，帧超时定时器时间固定为2mS*/
 }
 else
 {
          /* The timer reload value for a character is given by:
             *
             * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
             *             = 11 * Ticks_per_1s / Baudrate
             *             = 11000 / Baudrate
             * The reload for t3.5 is 3.5 times this value.
             */
 timer_frame_1ms=(7UL * 11000UL) /(2UL*baudrate ); 
 }
 APP_LOG_INFO("串口定时器值为%dmS.\r\n",timer_frame_1ms);
 if(xcomm_port_serial_timer_init(timer_frame_1ms)!=COMM_OK)
 {
 status=COMM_ERR;
 APP_LOG_ERROR("通信串口定时器初始化错误！\r\n");
 }
 return status;
 
}
/*获取一帧数据的地址和长度*/
static comm_status_t comm_receive_fsm(uint8_t **ptr_buff,uint8_t *ptr_recv_len)
{
  uint8_t addr;
  xcomm_port_serial_recv(ptr_buff,ptr_recv_len);
  
  addr=*ptr_buff[COMM_ADDR_OFFSET];
  if(addr!=comm_addr)
  {
   APP_LOG_WARNING("%d不是本机地址.不接收这帧数据.\r\n",addr);
   return COMM_ERR;
  }
  if(*ptr_recv_len<COMM_SERIAL_PDU_SIZE_MIN)
  {
   APP_LOG_WARNING("%d个帧数据少于最小串口数量.不接收这帧数据.\r\n",*ptr_recv_len);
   return COMM_ERR;
  }
  return COMM_OK;
}

/*发送一帧数据*/
static comm_status_t comm_send_fsm(uint8_t *ptr_buff,uint8_t send_len)
{
  uint8_t addr;
  addr=ptr_buff[COMM_ADDR_OFFSET];
  if(addr!=comm_addr)
  {
   APP_LOG_WARNING("%d不是本机地址.不发送这帧数据.\r\n",addr);
   return COMM_ERR;
  }
  if(send_len<COMM_SERIAL_PDU_SIZE_MIN)
  {
   APP_LOG_WARNING("%d个帧数据少于最小串口数量.不发送这帧数据.\r\n",send_len);
   return COMM_ERR;
  }
  xcomm_port_serial_send(ptr_buff,send_len);
  return COMM_OK;
}

/*与上位机的通信任务*/
void communication_task(void const * argument)
{
 osEvent signals;
 uint8_t *ptr_buff,recv_len,send_len;
 comm_status_t status;
 APP_LOG_INFO("######上位机通信任务开始.\r\n");
 status=comm_init(COMM_ADDR,COMM_PORT,COMM_BAUDRATE,COMM_DATABITS);
 if(status!=COMM_OK)
 {
 APP_LOG_ERROR("通信任务初始化失败.\r\n");
 }
 while(1)
 {
 signals=osSignalWait(COMM_ALL_SIGNALS,osWaitForever);
 if(signals.status!=osEventSignal)
 continue;
 /*处理通信缓存溢出信号*/
 if(signals.value.signals & COMM_BUFF_OVERFLOW_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到串口缓存溢出信号.\r\n"); 
 }
 /*处理接收数据帧信号*/
 if(signals.value.signals & COMM_RECV_FSM_SIGNAL)
 {
 APP_LOG_ERROR("通信任务收到串口数据帧信号.开始接收.\r\n");
 status=comm_receive_fsm(&ptr_buff,&recv_len);
 if(status==COMM_OK)
 {
  APP_LOG_ERROR("通信任务接收数据成功.\r\n"); 
  APP_LOG_ERROR("向通信任务发送解析协议信号.\r\n"); 
  osSignalSet(comm_task_hdl,COMM_PARSE_PROTOCOL_SIGNAL);
 }
 }
 /*处理解析数据帧信号*/
 if(signals.value.signals & COMM_PARSE_PROTOCOL_SIGNAL)
 {
 status=comm_protocol_parse(ptr_buff,recv_len,&send_len);
 if(status==COMM_OK)
 {
 APP_LOG_INFO("协议解析完成.\r\n");
 APP_LOG_INFO("向通信任务发送发送数据帧信号.\r\n");
 osSignalSet(comm_task_hdl,COMM_SEND_FSM_SIGNAL);
 }
 }
 /*处理发送数据帧信号*/
 if(signals.value.signals & COMM_SEND_FSM_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到发送串口数据帧信号.\r\n"); 
  APP_LOG_ERROR("使能串口发送.\r\n")
  send_len+=COMM_ADDR_SIZE;
  
  xcomm_port_serial_enable(FORTUNA_FALSE,FORTUNA_TRUE);
  comm_send_fsm(ptr_buff,send_len);
 }
 /*处理发送数据帧完毕信号*/
 if(signals.value.signals & COMM_SEND_FSM_OVER_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到发送串口数据帧完毕信号.\r\n"); 
  APP_LOG_ERROR("使能串口接收.禁止串口发送.\r\n")
  xcomm_port_serial_enable(FORTUNA_TRUE,FORTUNA_FALSE);
 }
}
}

/*解析数据帧*/
static comm_status_t comm_protocol_parse(uint8_t *ptr_buff,uint8_t recv_len,uint8_t *ptr_send_len)
{
 comm_status_t status;
 status=COMM_ERR;
 /*数据长度去掉地址一个字节,同时更新指针*/
 recv_len-=COMM_ADDR_SIZE;
 *ptr_send_len=COMM_ADDR_SIZE;
 APP_LOG_ERROR("通信任务收到解析协议信号.\r\n");
 if(recv_len<=COMM_PDU_SIZE_MIN)
 {
   APP_LOG_WARNING("有效数据过短.无效.\r\n");
   return status;
 } 
 for(uint8_t i=0;i<COMM_CMD_CNT;i++)
 {
 if(comm_cmd[i].cmd_code==ptr_buff[COMM_CMD_OFFSET] && comm_cmd[i].pdu_len==recv_len)
 {
   /*去除命令码的长度*/
   recv_len-=COMM_CMD_SIZE;
   /*发送长度再加上命令码的长度*/   
   *ptr_send_len+=COMM_CMD_SIZE;
   status=comm_cmd[i].comm_cmd_process(&ptr_buff[COMM_PARAM_OFFSET],recv_len,ptr_send_len);
   break;
 }
 }
 if(status!=COMM_OK)
 {
  APP_LOG_WARNING("协议解析.命令处理 出错.\r\n");
 }
 return status;
}

/*命令码0x01 去除皮重 处理函数*/
static comm_status_t comm_cmd01_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
  uint8_t scale;
  osEvent signal;
  scale_msg_t msg;
  APP_LOG_DEBUG("执行命令0x01.去皮指令.\r\n");
  if(param_len!=COMM_CMD01_PARAM_SIZE)
  {
   APP_LOG_ERROR("命令0x01参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
  }
  /*装载去皮指令参数*/
  scale=ptr_param[0];
  if(scale>COMM_CMD_PARAM_SCALE_MAX)
  {
   APP_LOG_ERROR("命令0x01参数%d非法.\r\n",scale);
   return COMM_ERR;
  }
  msg.type=COMM_CLEAR_SCALE_TARE_WEIGHT_MSG;
  msg.scale=scale;
  /*向电子秤任务发送去除皮重消息*/
  APP_LOG_DEBUG("向电子秤任务发送去除皮重消息.");
  osMessagePut(elec_scales_task_hdl,*(uint32_t*)&msg,0);
  /*等待处理返回*/
  APP_LOG_DEBUG("等待电子秤任务返回结果...");
  signal=osSignalWait(COMM_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL|COMM_CLEAR_SCALE_TARE_WEIGHT_ERR_SIGNAL,COMM_CLEAR_SCALE_TARE_WEIGHT_TIMEOUT);
  if(signal.status==osEventSignal && (signal.value.signals & COMM_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL))
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD01_EXECUTE_RESULT_SUCCESS; 
   APP_LOG_INFO("命令0x01执行成功.\r\n");
  }
  else
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD01_EXECUTE_RESULT_FAIL;
   APP_LOG_INFO("命令0x01执行失败.\r\n");
  }
  /*更新需要发送的数据长度*/
  *ptr_send_len+=COMM_CMD01_EXECUTE_RESULT_SIZE;

  return COMM_OK;
}
/*命令码0x02 校准 处理函数*/
static comm_status_t comm_cmd02_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t scale;
 uint16_t weight;
 scale_msg_t msg;
 osEvent signal;
 APP_LOG_DEBUG("执行命令0x02.校准指令.\r\n");
 if(param_len!=COMM_CMD02_PARAM_SIZE)
 {
  APP_LOG_ERROR("命令0x02参数长度不匹配.\r\n",param_len);
  return COMM_ERR;
 }
 /*装载校准指令参数*/
 scale=ptr_param[0];
 weight=(uint16_t)ptr_param[1]<<8;
 weight|=ptr_param[2];
 if(scale>COMM_CMD_PARAM_SCALE_MAX)
 {
  APP_LOG_ERROR("命令0x02参数%d非法.\r\n",scale);
  return COMM_ERR;
 }
 msg.type=COMM_CALIBRATE_SCALE_WEIGHT_MSG;
 msg.scale=scale;
 msg.param16=weight;
 /*向电子秤任务发送校准消息*/
 APP_LOG_DEBUG("向电子秤任务发送校准消息.");
 osMessagePut(elec_scales_task_hdl,*(uint32_t*)&msg,0);
 /*等待处理返回*/
 APP_LOG_DEBUG("等待电子秤任务返回结果...");
 signal=osSignalWait(COMM_CALIBRATE_SCALE_WEIGHT_OK_SIGNAL|COMM_CALIBRATE_SCALE_WEIGHT_ERR_SIGNAL,COMM_CALIBRATE_SCALE_WEIGHT_TIMEOUT);
 if(signal.status==osEventSignal && (signal.value.signals & COMM_CALIBRATE_SCALE_WEIGHT_OK_SIGNAL))
 {
  /*回填操作结果*/
  ptr_param[0]=COMM_CMD02_EXECUTE_RESULT_SUCCESS; 
  APP_LOG_INFO("命令0x02执行成功.\r\n");
 }
 else
 {
  /*回填操作结果*/
  ptr_param[0]=COMM_CMD02_EXECUTE_RESULT_FAIL;
  APP_LOG_INFO("命令0x02执行失败.\r\n");
 }
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD02_EXECUTE_RESULT_SIZE;
  return COMM_OK;
}
/*命令码0x03 查询重量 处理函数*/
static comm_status_t comm_cmd03_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t scale;
 uint16_t weight;
 APP_LOG_DEBUG("执行命令0x03.获取称重值.\r\n");
 if(param_len!=COMM_CMD03_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x03参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 /*装载查询重量指令参数*/
 scale=ptr_param[0];
 if(scale>COMM_CMD_PARAM_SCALE_MAX)
 {
  APP_LOG_ERROR("命令0x03参数%d非法.\r\n",scale);
  return COMM_ERR;
 }
 APP_LOG_DEBUG("获取称重值的称号：%d\r\n",scale);
 /*获取所有20个称重量*/
 if(scale==0)
 {
 for(uint8_t i=0;i<COMM_VIRTUAL_SCALE_MAX;i++)
 {
 /*回填重量值*/
  weight=scale_weight[i];
  ptr_param[i*2]=weight>>8;
  ptr_param[i*2+1]=weight & 0xff;
 }
 APP_LOG_DEBUG("获取称重值：#1:%dg #2:%dg #3:%dg #4:%dg\r\n",ptr_param[0]<<8|ptr_param[1],\
   ptr_param[2]<<8|ptr_param[3],ptr_param[4]<<8|ptr_param[5],ptr_param[6]<<8|ptr_param[7]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD03_EXECUTE_RESULT_ALL_SCALES_SIZE;
 }
 else
 {
  weight=scale_weight[scale-1];
  ptr_param[0]=weight>>8;
  ptr_param[1]=weight & 0xff;
  APP_LOG_DEBUG("获取称重值：%d\r\n",ptr_param[0]<<8|ptr_param[1]);
  /*更新需要发送的数据长度*/
  *ptr_send_len+=COMM_CMD03_EXECUTE_RESULT_ONE_SCALE_SIZE;
 }
  return COMM_OK;
} 
/*命令码04 查询称重单元数量 处理函数*/
static comm_status_t comm_cmd04_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t scale_cnt;
 APP_LOG_DEBUG("执行命令04.查询称重单元\r\n");
 if(param_len!=COMM_CMD04_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令04参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 /*回填称重数量*/
 scale_cnt=elec_scale_cnt;
 ptr_param[0]=scale_cnt;
 APP_LOG_DEBUG("获取的称重单元数量：%d\r\n",ptr_param[0]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD04_EXECUTE_RESULT_SIZE;
 return COMM_OK;
}

/*命令码0x11 查询门的状态 处理函数*/
static comm_status_t comm_cmd11_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 obj_status_t status; 
 APP_LOG_DEBUG("执行命令0x11.查询门的状态.\r\n");
 if(param_len!=COMM_CMD11_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x11参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 status=door_status;
 /*回填称重数量*/
 ptr_param[0]=status;
 APP_LOG_DEBUG("获取的门的状态：%d\r\n", ptr_param[0]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD11_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}

/*命令码21 开锁 处理函数*/
static comm_status_t comm_cmd21_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
  osEvent signal;
  scale_msg_t msg;
  APP_LOG_DEBUG("执行命令0x21.开锁指令.\r\n");
  if(param_len!=COMM_CMD01_PARAM_SIZE)
  {
   APP_LOG_ERROR("命令0x21参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
  }
  msg.type=COMM_UNLOCK_LOCK_MSG;
  /*向锁任务发送开锁消息*/
  APP_LOG_DEBUG("向锁任务发送开锁消息.\r\n");
  osMessagePut(lock_task_hdl,*(uint32_t*)&msg,0);
  /*等待处理返回*/
  APP_LOG_DEBUG("等待锁任务返回结果...");
  signal=osSignalWait(COMM_UNLOCK_LOCK_OK_SIGNAL|COMM_UNLOCK_LOCK_ERR_SIGNAL,COMM_UNLOCK_LOCK_TIMEOUT);
  if(signal.status==osEventSignal && (signal.value.signals & COMM_UNLOCK_LOCK_OK_SIGNAL))
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD21_EXECUTE_RESULT_SUCCESS;
   APP_LOG_DEBUG("命令0x21开锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_INFO("命令0x21执行成功.\r\n");
  }
  else
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD21_EXECUTE_RESULT_FAIL;
   APP_LOG_DEBUG("命令0x21开锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_INFO("命令0x21执行失败.\r\n");
  }
  /*更新需要发送的数据长度*/
  *ptr_send_len+=COMM_CMD21_EXECUTE_RESULT_SIZE;

  return COMM_OK;
}

/*命令码22 关锁 处理函数*/
static comm_status_t comm_cmd22_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
  osEvent signal;
  scale_msg_t msg;
  APP_LOG_DEBUG("执行命令0x22.关锁指令.\r\n");
  if(param_len!=COMM_CMD01_PARAM_SIZE)
  {
   APP_LOG_ERROR("命令0x22参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
  }

  msg.type=COMM_LOCK_LOCK_MSG;
  /*向锁任务发送关锁消息*/
  APP_LOG_DEBUG("向锁任务发送关锁消息.\r\n");
  osMessagePut(lock_task_hdl,*(uint32_t*)&msg,0);
  /*等待处理返回*/
  APP_LOG_DEBUG("等待锁任务返回结果...");
  signal=osSignalWait(COMM_LOCK_LOCK_OK_SIGNAL|COMM_LOCK_LOCK_ERR_SIGNAL,COMM_LOCK_LOCK_TIMEOUT);
  if(signal.status==osEventSignal && (signal.value.signals & COMM_LOCK_LOCK_OK_SIGNAL))
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD22_EXECUTE_RESULT_SUCCESS; 
   APP_LOG_DEBUG("命令0x22关锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_INFO("命令0x22执行成功.\r\n");
  }
  else
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD22_EXECUTE_RESULT_FAIL;
   APP_LOG_DEBUG("命令0x22关锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_INFO("命令0x22执行失败.\r\n");
  }
  /*更新需要发送的数据长度*/
  *ptr_send_len+=COMM_CMD22_EXECUTE_RESULT_SIZE;

  return COMM_OK;
}

/*命令码0x23 查询锁的状态 处理函数*/
static comm_status_t comm_cmd23_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 obj_status_t status; 
 APP_LOG_DEBUG("执行命令0x23.查询门的状态.\r\n");
 if(param_len!=COMM_CMD11_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x23参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 status=lock_status;
 /*回填称重数量*/
 ptr_param[0]=status;
 APP_LOG_DEBUG("获取的锁的状态：%d\r\n",status);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD23_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}
/*命令码0x31 查询UPS的状态 处理函数*/
static comm_status_t comm_cmd31_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 obj_status_t status; 
 APP_LOG_DEBUG("执行命令0x31.查询门的状态.\r\n");
 if(param_len!=COMM_CMD31_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x31参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 status=ups_status;
 /*回填称重数量*/
 ptr_param[0]=status;
 APP_LOG_DEBUG("获取的ups的状态：%d\r\n",ptr_param[0]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD31_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}

/*命令码0x41 查询温度 处理函数*/
static comm_status_t comm_cmd41_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 int8_t t; 
 APP_LOG_DEBUG("执行命令0x41.查询温度.\r\n");
 if(param_len!=COMM_CMD41_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x41参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 t=temperature;
 /*回填称重数量*/
 ptr_param[0]=t;
 APP_LOG_DEBUG("获取的温度值：%d\r\n",t);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD41_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}

/*命令码0x51 查询id和版本号 处理函数*/
static comm_status_t comm_cmd51_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t id,ver;
 APP_LOG_DEBUG("执行命令0x51.查询厂商id和固件版本号.\r\n");
 if(param_len!=COMM_CMD51_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x51参数长度不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 id=vendor_id;
 ver=firmware_version;
 /*回填称重数量*/
 ptr_param[0]=id;
 ptr_param[1]=ver;
 APP_LOG_DEBUG("获取的厂商id：%d.固件版本号：%d.\r\n",ptr_param[0],ptr_param[1]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD51_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}


