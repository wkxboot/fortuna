#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "fortuna_common.h"
#include "scales.h"
#include "comm_protocol.h"
#include "host_comm_task.h"
#include "scale_func_task.h"
#include "comm_port_serial.h"
#include "comm_port_timer.h"
#include "scale_func_task.h"
#include "lock_task.h"
#include "ups_task.h"
#include "temperature_task.h"
#define APP_LOG_MODULE_NAME   "[protocol]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

uint8_t timer_frame_1ms;
uint8_t comm_addr;

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

static volatile uint8_t rx_buff[BUFF_SIZE_MAX];
static volatile uint8_t *ptr_send_buff;/*发送数据时缓存位置指针*/
static volatile uint8_t recv_cnt,send_cnt;/*发送和接收的数据大小*/

uint8_t comm_addr;

void HOST_PROTOCOL_CRITICAL_REGION_ENTER()      
{                               
  if(__get_IPSR()==0)                  
  {                                  
   taskENTER_CRITICAL();                                              
 }  
}

void HOST_PROTOCOL_CRITICAL_REGION_EXIT()   
{                                  
  if ( __get_IPSR()==0)                 
  {                                  
   taskEXIT_CRITICAL();              
  }                                  
}

void comm_fsm_timer_expired()
{
 /*关闭串口接收，方便后续数据处理*/
 APP_LOG_DEBUG("串口定时器到期.\r\n")
 APP_LOG_DEBUG("接收完一帧数据.向通信任务发送信号.\r\n");
 /*禁止串口发送接收*/
 xcomm_port_serial_enable(FORTUNA_FALSE,FORTUNA_FALSE); 
 /*发送接收完成信号*/
 osSignalSet(host_comm_task_hdl,HOST_COMM_TASK_RECV_FSM_SIGNAL);
}


comm_status_t comm_init(uint8_t slave_addr,uint8_t port,uint32_t baudrate,uint8_t databits)
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
 APP_LOG_DEBUG("串口定时器值为%dmS.\r\n",timer_frame_1ms);
 if(xcomm_port_serial_timer_init(timer_frame_1ms)!=COMM_OK)
 {
 status=COMM_ERR;
 APP_LOG_ERROR("通信串口定时器初始化错误！\r\n");
 }
 /*等待新的数据*/
 xcomm_port_serial_enable(FORTUNA_TRUE,FORTUNA_FALSE); 
 return status;
 
}


void comm_byte_receive(void)
{
 uint8_t recv_byte;
 xcomm_port_serial_get_byte(&recv_byte);
 if(recv_cnt>=BUFF_SIZE_MAX)
 {
 APP_LOG_ERROR("串口缓存溢出.本次数据无效.\r\n");
 recv_cnt=0;
 }
 rx_buff[recv_cnt++]=recv_byte;
 /*重新开始定时器*/
 xcomm_port_serial_timer_start();
}

void comm_byte_send(void)
{
if(send_cnt!=0)
{
xcomm_port_serial_send_byte(*ptr_send_buff); 
ptr_send_buff++;/*下一个字节*/
send_cnt--;/*更新待发送数量*/
}
else
{
 /*等待新的数据*/
 xcomm_port_serial_enable(FORTUNA_TRUE,FORTUNA_FALSE); 
 osSignalSet(host_comm_task_hdl,HOST_COMM_TASK_SEND_FSM_OVER_SIGNAL);  
}
}


/*获取一帧数据的地址和长度*/
comm_status_t comm_receive_fsm(uint8_t **ptr_buff,uint8_t *ptr_recv_len)
{ 
  HOST_PROTOCOL_CRITICAL_REGION_ENTER();
  /*接收数据长度和缓存*/
  *ptr_recv_len=recv_cnt;;
  *ptr_buff=(uint8_t*)rx_buff; 
  recv_cnt=0;/*重新开始计数*/
  HOST_PROTOCOL_CRITICAL_REGION_EXIT();
  return COMM_OK;
}

/*发送一帧数据*/
comm_status_t comm_send_fsm(uint8_t *ptr_buff,uint8_t send_len)
{
  HOST_PROTOCOL_CRITICAL_REGION_ENTER();
  /*数据长度加上地址长度*/
  ptr_buff[COMM_ADDR_OFFSET]=comm_addr;
  send_cnt=send_len;
  ptr_send_buff=ptr_buff;
  HOST_PROTOCOL_CRITICAL_REGION_EXIT();
  /*启动发送*/
  xcomm_port_serial_enable(FORTUNA_FALSE,FORTUNA_TRUE); 
  return COMM_OK;
}

/*解析数据帧*/
comm_status_t comm_protocol_parse(uint8_t *ptr_buff,uint8_t recv_len,uint8_t *ptr_send_len)
{
 comm_status_t status;
 uint8_t addr;
 status=COMM_OK;
 /*发送的数据长度置0*/
 *ptr_send_len=0;
  APP_LOG_DEBUG("解析协议...\r\n");
  addr=ptr_buff[COMM_ADDR_OFFSET];
  if(addr!=comm_addr) 
  {
  APP_LOG_ERROR("%d不是本机地址.\r\n",addr);
  status= COMM_ERR;
  }
  if(recv_len<COMM_SERIAL_PDU_SIZE_MIN)
  {
   APP_LOG_ERROR("%d个帧数据少于串口最小接收值.\r\n",recv_len);
   status= COMM_ERR;
  }
 /*发生了错误*/
  if(status!=COMM_OK)
  {
  goto parse_err_handle;
  }
 /*处理发送长度和接收长度*/
 recv_len-=COMM_ADDR_SIZE;
 *ptr_send_len+=COMM_ADDR_SIZE;
 
 for(uint8_t i=0;i<COMM_CMD_CNT;i++)
 {
 status=COMM_ERR;
 if(comm_cmd[i].cmd_code==ptr_buff[COMM_CMD_OFFSET])
 {
   /*去除命令码的长度*/
   recv_len-=COMM_CMD_SIZE;
   /*发送的数据长度再加上命令码的长度*/   
   *ptr_send_len+=COMM_CMD_SIZE;
   status=comm_cmd[i].comm_cmd_process(&ptr_buff[COMM_PARAM_OFFSET],recv_len,ptr_send_len);
   break;
 }
 }
if(status==COMM_OK)
return status;

APP_LOG_ERROR("协议数据错误.\r\n");

parse_err_handle:
  /*等待新数据*/
 xcomm_port_serial_enable(FORTUNA_TRUE,FORTUNA_FALSE); 
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
   APP_LOG_ERROR("命令0x01参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
  }
  /*装载去皮指令参数*/
  scale=ptr_param[0];
  if(scale>COMM_CMD_PARAM_SCALE_MAX)
  {
   APP_LOG_ERROR("命令0x01参数%d非法.\r\n",scale);
   return COMM_ERR;
  }
  msg.type=SCALE_FUNC_TASK_CLEAR_TARE_WEIGHT_MSG;
  msg.scale=scale;
  msg.param16=1;/*非0代表当前毛重为皮重值*/
  /*向电子秤任务发送去除皮重消息*/
  APP_LOG_DEBUG("向电子秤任务发送去除皮重消息.\r\n");
  osMessagePut(scale_func_msg_q_id,*(uint32_t*)&msg,0);
  /*等待处理返回*/
  APP_LOG_DEBUG("等待电子秤任务返回结果...\r\n");
  signal=osSignalWait(COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL|COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_ERR_SIGNAL,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_TIMEOUT);
  if(signal.status==osEventSignal && (signal.value.signals & COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL))
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD01_EXECUTE_RESULT_SUCCESS; 
   APP_LOG_DEBUG("命令0x01执行成功.\r\n");
  }
  else
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD01_EXECUTE_RESULT_FAIL;
   APP_LOG_ERROR("命令0x01执行失败.\r\n");
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
  APP_LOG_ERROR("命令0x02参数长度%d不匹配.\r\n",param_len);
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
/*向电子秤任务发送校准消息*/
 APP_LOG_DEBUG("向电子秤任务发送校准消息.\r\n");
 APP_LOG_DEBUG("第一步发送设置皮重为0消息.\r\n");
  /*设置皮重为0*/
 msg.type=SCALE_FUNC_TASK_CLEAR_TARE_WEIGHT_MSG;
 msg.scale=scale;
 msg.param16=0;
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&msg,0);
  /*等待处理返回*/
 APP_LOG_DEBUG("等待电子秤任务返回设置0皮重结果...\r\n");
 signal=osSignalWait(COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL|COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_ERR_SIGNAL,COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_TIMEOUT);
 if(!(signal.status==osEventSignal && (signal.value.signals & COMM_TASK_CLEAR_SCALE_TARE_WEIGHT_OK_SIGNAL)))
 goto comm_calibrate_err_handle;
 
 APP_LOG_DEBUG("第二步发送标定内码消息.\r\n");
 /*标定内码*/
 msg.type=SCALE_FUNC_TASK_CALIBRATE_CODE_MSG;
 msg.scale=scale;
 msg.param16=weight;
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&msg,0);
 /*等待处理返回*/
 APP_LOG_DEBUG("等待电子秤任务返回标定内码结果...\r\n");
 signal=osSignalWait(COMM_TASK_CALIBRATE_SCALE_CODE_OK_SIGNAL|COMM_TASK_CALIBRATE_SCALE_CODE_ERR_SIGNAL,COMM_TASK_CALIBRATE_SCALE_CODE_TIMEOUT);
 if(!(signal.status==osEventSignal && (signal.value.signals & COMM_TASK_CALIBRATE_SCALE_CODE_OK_SIGNAL)))
 goto comm_calibrate_err_handle;
 
 APP_LOG_DEBUG("第三步发送标定测量值消息.\r\n");
 msg.type=SCALE_FUNC_TASK_CALIBRATE_MEASUREMENT_MSG;
 msg.scale=scale;
 msg.param16=weight;
 osMessagePut(scale_func_msg_q_id,*(uint32_t*)&msg,0);
 /*等待处理返回*/
 APP_LOG_DEBUG("等待电子秤任务返回标定测量值结果...\r\n");
 signal=osSignalWait(COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_OK_SIGNAL|COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_ERR_SIGNAL,COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_TIMEOUT);
 if(!(signal.status==osEventSignal && (signal.value.signals & COMM_TASK_CALIBRATE_SCALE_MEASUREMENT_OK_SIGNAL)))
 goto comm_calibrate_err_handle;
 
 /*回填操作结果*/
 ptr_param[0]=COMM_CMD02_EXECUTE_RESULT_SUCCESS; 
 APP_LOG_DEBUG("命令0x02执行成功.\r\n");
 goto comm_calibrate_ok_handle;
  
comm_calibrate_err_handle:
  /*回填操作结果*/
  ptr_param[0]=COMM_CMD02_EXECUTE_RESULT_FAIL;
  APP_LOG_ERROR("命令0x02执行失败.\r\n");
  
comm_calibrate_ok_handle:
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD02_EXECUTE_RESULT_SIZE;
  return COMM_OK;
}
/*命令码0x03 查询重量 处理函数*/
static comm_status_t comm_cmd03_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t scale;
 int32_t weight;
 APP_LOG_DEBUG("执行命令0x03.获取称重值.\r\n");
 if(param_len!=COMM_CMD03_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x03参数长度%d不匹配.\r\n",param_len);
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
  get_net_weight(i+1,&weight);
  if(weight>0x7fff)
  {
  ptr_param[i*2]=0x7f;
  ptr_param[i*2+1]=0xff;
  }
  else if(weight<(-0x8000))
  {
  ptr_param[i*2]=0x80;
  ptr_param[i*2+1]=00;
  }
  else
  {
  ptr_param[i*2]=weight>>8;
  ptr_param[i*2+1]=weight; 
  }
 }
 APP_LOG_DEBUG("获取称重值：#1:%dg #2:%dg #3:%dg #4:%dg\r\n",(int16_t)(ptr_param[0]<<8|ptr_param[1]),\
  (int16_t)(ptr_param[2]<<8|ptr_param[3]),(int16_t)(ptr_param[4]<<8|ptr_param[5]),(int16_t)(ptr_param[6]<<8|ptr_param[7]));
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD03_EXECUTE_RESULT_ALL_SCALES_SIZE;
 }
 else
 {
  get_net_weight(scale,&weight);
  if(weight>0x7fff)
  {
  ptr_param[0]=0x7f;
  ptr_param[1]=0xff;
  }
  else if(weight<(-0x8000))
  {
  ptr_param[0]=0x80;
  ptr_param[1]=00;
  }
  else
  {
  ptr_param[0]=weight>>8;
  ptr_param[1]=weight & 0xff;
  }
  APP_LOG_DEBUG("获取称重值：%d\r\n",(int16_t)(ptr_param[0]<<8|ptr_param[1]));
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
   APP_LOG_ERROR("命令04参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 /*回填称重数量*/
 scale_cnt=SCALES_CNT_MAX;
 ptr_param[0]=scale_cnt;
 APP_LOG_DEBUG("获取的称重单元数量：%d\r\n",ptr_param[0]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD04_EXECUTE_RESULT_SIZE;
 return COMM_OK;
}

/*命令码0x11 查询门的状态 处理函数*/
static comm_status_t comm_cmd11_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t state; 
 APP_LOG_DEBUG("执行命令0x11.查询门的状态.\r\n");
 if(param_len!=COMM_CMD11_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x11参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 state=get_door_state();;
 /*回填门的状态*/
 ptr_param[0]=state;
 APP_LOG_DEBUG("获取的门的状态：%d\r\n", ptr_param[0]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD11_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}

/*命令码21 开锁 处理函数*/
static comm_status_t comm_cmd21_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
  osEvent signal;
  lock_msg_t msg;
  APP_LOG_DEBUG("执行命令0x21.开锁指令.\r\n");
  if(param_len!=COMM_CMD21_PARAM_SIZE)
  {
   APP_LOG_ERROR("命令0x21参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
  }
  msg.type=LOCK_TASK_UNLOCK_LOCK_MSG;
  /*向锁任务发送开锁消息*/
  APP_LOG_DEBUG("向锁任务发送开锁消息.\r\n");
  osMessagePut(lock_task_msg_q_id,*(uint32_t*)&msg,0);
  /*等待处理返回*/
  APP_LOG_DEBUG("等待锁任务返回结果...\r\n");
  signal=osSignalWait(COMM_TASK_UNLOCK_LOCK_OK_SIGNAL|COMM_TASK_UNLOCK_LOCK_ERR_SIGNAL,COMM_TASK_UNLOCK_LOCK_TIMEOUT);
  if(signal.status==osEventSignal && (signal.value.signals & COMM_TASK_UNLOCK_LOCK_OK_SIGNAL))
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD21_EXECUTE_RESULT_SUCCESS;
   APP_LOG_DEBUG("命令0x21开锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_DEBUG("命令0x21执行成功.\r\n");
  }
  else
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD21_EXECUTE_RESULT_FAIL;
   APP_LOG_ERROR("命令0x21开锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_ERROR("命令0x21执行失败.\r\n");
  }
  /*更新需要发送的数据长度*/
  *ptr_send_len+=COMM_CMD21_EXECUTE_RESULT_SIZE;

  return COMM_OK;
}

/*命令码22 关锁 处理函数*/
static comm_status_t comm_cmd22_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
  osEvent signal;
  lock_msg_t msg;
  APP_LOG_DEBUG("执行命令0x22.关锁指令.\r\n");
  if(param_len!=COMM_CMD22_PARAM_SIZE)
  {
   APP_LOG_ERROR("命令0x22参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
  }
  msg.type=LOCK_TASK_LOCK_LOCK_MSG;
  /*向锁任务发送关锁消息*/
  APP_LOG_DEBUG("向锁任务发送关锁消息.\r\n");
  osMessagePut(lock_task_msg_q_id,*(uint32_t*)&msg,0);
  /*等待处理返回*/
  APP_LOG_DEBUG("等待锁任务返回结果...\r\n");
  signal=osSignalWait(COMM_TASK_LOCK_LOCK_OK_SIGNAL|COMM_TASK_LOCK_LOCK_ERR_SIGNAL,COMM_TASK_LOCK_LOCK_TIMEOUT);
  if(signal.status==osEventSignal && (signal.value.signals & COMM_TASK_LOCK_LOCK_OK_SIGNAL))
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD22_EXECUTE_RESULT_SUCCESS; 
   APP_LOG_DEBUG("命令0x22关锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_DEBUG("命令0x22执行成功.\r\n");
  }
  else
  {
   /*回填操作结果*/
   ptr_param[0]=COMM_CMD22_EXECUTE_RESULT_FAIL;
   APP_LOG_ERROR("命令0x22关锁执行的状态：%d\r\n", ptr_param[0]);
   APP_LOG_ERROR("命令0x22执行失败.\r\n");
  }
  /*更新需要发送的数据长度*/
  *ptr_send_len+=COMM_CMD22_EXECUTE_RESULT_SIZE;

  return COMM_OK;
}

/*命令码0x23 查询锁的状态 处理函数*/
static comm_status_t comm_cmd23_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t state; 
 APP_LOG_DEBUG("执行命令0x23.查询锁的状态.\r\n");
 if(param_len!=COMM_CMD23_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x23参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 state=get_lock_state();
 /*回填锁的状态*/
 ptr_param[0]=state;
 APP_LOG_DEBUG("获取的锁的状态：%d\r\n",ptr_param[0]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD23_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}
/*命令码0x31 查询UPS的状态 处理函数*/
static comm_status_t comm_cmd31_process(uint8_t *ptr_param,uint8_t param_len,uint8_t *ptr_send_len) 
{
 uint8_t state; 
 APP_LOG_DEBUG("执行命令0x31.查询UPS的状态.\r\n");
 if(param_len!=COMM_CMD31_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令0x31参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 state=get_ups_state();
 /*回填称重数量*/
 ptr_param[0]=state;
 APP_LOG_DEBUG("获取的UPS的状态：%d\r\n",ptr_param[0]);
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
   APP_LOG_ERROR("命令0x41参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 t=get_average_temperature();
 /*回填温度*/
 ptr_param[0]=t;
 APP_LOG_DEBUG("获取的温度值：%d\r\n",ptr_param[0]);
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
   APP_LOG_ERROR("命令0x51参数长度%d不匹配.\r\n",param_len);
   return COMM_ERR;
 }
 id=VENDOR_ID_CHANGHONG;
 ver=FIRMWARE_VERSION;
 /*回填称重数量*/
 ptr_param[0]=id;
 ptr_param[1]=ver;
 APP_LOG_DEBUG("获取的厂商id：%d.固件版本号：%d.\r\n",ptr_param[0],ptr_param[1]);
 /*更新需要发送的数据长度*/
 *ptr_send_len+=COMM_CMD51_EXECUTE_RESULT_SIZE;

 return COMM_OK;
}


