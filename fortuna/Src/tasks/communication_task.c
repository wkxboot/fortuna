#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#define APP_LOG_MODULE_NAME   "[comm]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

typedef enum
{
FORTUNA_TRUE=1;
FORTUNA_FALSE=0;
}fortuna_bool_t;

typedef enum
{
COMM_OK=0;
COMM_ERR;
}comm_status_t;



#define  COMM_BUFF_MAX            44
#define  comm_buff[COMM_BUFF_MAX];


static volatile uint8_t serial_recv_cnt;
uint8_t timer_frame_1ms;
uint8_t comm_addr;

typedef struct
{
  comm_status_t (*comm_cmd_process)(uint8_t *ptr_buff,uint8_t *ptr_len);
  uint8_t cmd_code;
  uint8_t len;
}comm_cmd_t;
#define  COMM_CMD_CNT       8
comm_cmd_t comm_cmd[COMM_CMD_CNT];

void comm_serial_isr(void)
{
 uint8_t recv_byte;
 comm_serial_get_byte(&recv_byte);
 if(serial_recv_cnt>=COMM_BUFF_MAX)
 {
 APP_LOG_ERROR("串口缓存溢出.本次数据无效.\r\n");
 osSignalSet(comm_task_hdl,COMM_BUFF_OVERFLOW_SIGNAL);
 serial_recv_cnt=0;
 }
 comm_buff[serial_recv_cnt++]=recv_byte;
}

void comm_fsm_timer_expired()
{
 /*关闭串口接收，方便后续数据处理*/
 APP_LOG_INFO("串口定时器到期.\r\n")
 APP_LOG_INFO("关闭串口接收.");
 comm_enable_serial(FORTUNA_FALSE,FORTUNA_FALSE);
 APP_LOG_INFO("接收完一帧数据.向通信任务发送信号.\r\n");
 /*发送接收完成信号*/
 comm_send_signal(COMM_RECV_FSM_SIGNAL);
}


comm_status_t comm_init(uint8_t slave_addr,uint8_t port,uint32_t baudrate,uint8_t databits)
{
/*本机地址*/
 comm_addr=slave_addr;
 comm_status_t status=FORTUNA_OK;
 
if(comm_serial_init(port,baudrate,databits)!=FORTUNA_OK)
 {
 err_code=FORTUNA_ERR;
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
 if(comm_serial_timer_init(timer_frame_1ms)!=FORTUNA_OK)
 {
 err_code=FORTUNA_ERR;
 APP_LOG_ERROR("通信串口定时器初始化错误！\r\n");
 }
 return err_code;
 
}

void communication_task(void const * argument)
{
 osEvent signals;
 uint8_t *ptr_buff;
 uint8_t addr;
 comm_status_t status;
 while(1)
 {
 signals=osSignalWait(COMM_ALL_SIGNALS,osWaitForever);
 if(signals.status!=osEventSignal)
 continue;
 
 if(signals.value.signals & COMM_BUFF_OVERFLOW_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到串口缓存溢出信号.\r\n"); 
 }
 if(signals.value.signals & COMM_RECV_FSM_SIGNAL)
 {
 APP_LOG_ERROR("通信任务收到串口数据帧信号.开始接收.\r\n");
 status=comm_receive(&ptr_buff,&comm_recv_cnt);
 if(status==FORTUNA_OK)
 {
  APP_LOG_ERROR("通信任务接收数据成功.\r\n"); 
  APP_LOG_ERROR("向通信任务发送解析协议信号.\r\n"); 
  osSignalSet(comm_task_hdl,COMM_PARSE_PROTOCOL_SIGNAL);
 }
 }
 if(signals.value.signals & COMM_PARSE_PROTOCOL_SIGNAL)
 {
 addr=ptr_buff[COMM_ADDR_OFFSET];
 if(addr!=comm_addr)
 {
   APP_LOG_WARNING("%d不是本机地址.无效不解析协议.\r\n",addr);
   continue; 
 }
/*数据长度去掉地址一个字节,同时更新指针*/
 comm_recv_len-=COMM_ADDR_SIZE;
 ptr_buff=ptr_buff+COMM_ADDR_SIZE;
 APP_LOG_ERROR("通信任务收到解析协议信号.\r\n");
 status=comm_protocol_parse(&ptr_buff,&comm_recv_cnt,&comm_send_cnt);
 if(status==FORTUNA_OK)
 {
 APP_LOG_INFO("协议解析完成.\r\n");
 APP_LOG_INFO("向通信任务发送发送数据帧信号.\r\n");
 osSignalSet(comm_task_hdl,COMM_SEND_FSM_SIGNAL);
 }
 }
 if(signals.value.signals & COMM_SEND_FSM_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到发送串口数据帧信号.\r\n"); 
  APP_LOG_ERROR("使能串口发送.\r\n")
  comm_serial_enable(FORTUNA_FALSE,FORTUNA_TRUE);
  comm_serial_send_fsm(comm_buff,&comm_send_cnt);
 }
 if(signals.value.signals & COMM_SEND_OVER_SIGNAL)
 {
  APP_LOG_ERROR("通信任务收到发送串口数据帧完毕信号.\r\n"); 
  APP_LOG_ERROR("使能串口接收.禁止串口发送.\r\n")
  comm_serial_enable(FORTUNA_TRUE,FORTUNA_FALSE);
 }
}
}


comm_status_t comm_protocol_parse(uint8_t **ptr_buff,uint8_t *ptr_recv_len，uint8_t *ptr_send_len)
{
 fortuna_status_t status;
 uint8_t addr;

 status=FORTUNA_ERR;
 if(*ptr_recv_len<=COMM_PDU_LEN_MIN)
 {
   APP_LOG_WARNING("有效数据过短.无效.\r\n");
   return status;
 } 
 for(uint8_t i=0;i<COMM_CMD_CNT;i++)
 {
 if(comm_cmd[i].cmd_code== *ptr_buff[COMM_CMD_OFFSET] && comm_cmd[i].len==*ptr_recv_len)
 {
   /*去除命令码的长度*/
   *ptr_buff=*ptr_buff+COMM_CMD_SIZE;
   *ptr_recv_len=*ptr_recv_len-COMM_CMD_SIZE;
   /*发送长度先加上命令码的长度*/   
   *ptr_send_len=COMM_CMD_SIZE;
   status=comm_cmd[i].comm_cmd_process(ptr_buff,ptr_recv_len,ptr_send_len);
   break;
 }
 }
 if(status!=FORTUNA_OK)
 {
  APP_LOG_WARNING("协议解析.命令处理 出错.\r\n");
 }
 return status;
}

/*命令码01 去除皮重 处理函数*/
comm_status_t comm_cmd01_process(uint8_t **ptr_buff,uint8_t *ptr_recv_len，uint8_t *ptr_send_len) 
{
 uint8_t param;
 osEvent signal;
 uint32_t send_signal,wait_signal,timeout;
 if(*ptr_recv_len!=COMM_CMD01_PARAM_SIZE)
 {
   APP_LOG_ERROR("命令01参数长度不匹配.\r\n",addr);
   return COMM_ERR;
 }
 /*装载去皮指令参数*/
 param=*ptr_buff[0];
 if(param>COMM_CMD01_PARAM_MAX)
 {
   APP_LOG_ERROR("命令01参数%d非法.\r\n",param);
   return COMM_ERR;
 }
  switch(param)
  {
  case 0:/*对所有称去皮*/
  send_signal=COMM_CLEAR_ALL_TAR_WEIGHT_SIGNAL;
  break;
  case 1:/*1号称*/
  send_signal=COMM_CLEAR_SCALES1_TAR_WEIGHT_SIGNAL;
  break;
  case 2:/*2号称*/
  send_signal=COMM_CLEAR_SCALES2_TAR_WEIGHT_SIGNAL;
  break;
  case 3:/*3号称*/
  send_signal=COMM_CLEAR_SCALES3_TAR_WEIGHT_SIGNAL;
  break;
  case 4:/*4号称*/
  send_signal=COMM_CLEAR_SCALES4_TAR_WEIGHT_SIGNAL;
  break;
  default:
  APP_LOG_ERROR("命令01参数%d非法.\r\n",param);  
  }
  wait_signal=COMM_CLEAR_TAR_WEIGHT_OK_SIGNAL;
  timeout=COMM_CLEAR_TAR_TIMEOUT;
  osSignalSet(elec_scales_task_hdl,send_signal);
  signal=osSignalWait(wait_signal,timeout);
  if(signal.status==osEventSignal && (signal.value.signals & wait_signal))
  {
   /*回填操作结果*/
   *ptr_buff[0]=COMM_CMD01_EXECUTE_SUCCESS; 
   APP_LOG_INFO("命令01执行成功.\r\n",param);
  }
  else
  {
   /*回填操作结果*/
   *ptr_buff[0]=COMM_CMD01_EXECUTE_FAIL;
   APP_LOG_INFO("命令01执行成功.\r\n",param);
  }

  /*回填需要发送的数据长度*/
  *ptr_send_len=*ptr_send_len+COMM_CMD01_EXECUTE_RESULT_SIZE;

  return COMM_OK;
}
 
 