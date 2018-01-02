#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#define APP_LOG_MODULE_NAME   "[comm]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"




#define  COMM_ADDR_OFFSET             0
#define  COMM_ADDR_SIZE               1

#define  COMM_CMD_OFFSET              1
#define  COMM_CMD_SIZE                1

#define  COMM_PARAM_OFFSET            2


#define  COMM_BUFF_MAX                42


#define  COMM_RECV_MAX                COMM_BUFF_MAX
#define  COMM_BUFF_MIN                2


static uint8_t volatile comm_serial_buff[COMM_BUFF_MAX];
static uint8_t volatile *ptr_send_buff;
static uint8_t volatile comm_recv_cnt,comm_send_cnt;
static uint16_t timer_frame_1ms=0;
static uint8_t comm_addr;
typedef enum
{
  COMM_BOOL_TRUE =1;
  COMM_BOOL_FALSE=0;
}comm_bool_t;
typedef enum 
{
 COMM_ERR_NO_ERR;/*没有错误*/
 COMM_ERR_PORT_ERR;/*移植错误*/
 COMM_ERR_IO_ERR;/*通信端口错误*/
 COMM_ERR_INVAL_ERR;/*非法参数错误*/
 COMM_ERR_TIMEOUT_ERR;/*超时错误*/
}comm_err_code_t;
typedef enum
{
  COMM_RECV_STATE_INIT;
  COMM_RECV_STATE_IDLE;
  COMM_RECV_STATE_RECV;
  COMM_RECV_STATE_ERR;
}comm_recv_state_t;

comm_recv_state_t comm_recv_state;

typedef enum
{
  COMM_SEND_STATE_IDLE;
  COMM_SEND_STATE_SEND;
}comm_send_state_t;
comm_err_code_t comm_serial_init(uint8_t slav_addr,uint8_t port,uint32_t baudrate,uint8_t databits)
{
 comm_err_code_t err_code=COMMU_ERR_NO_ERR;
 slave_addr=addr;
 if(xcomm_port_serial_init(port,baudrate,databits)==COMM_FALSE)
 {
 err_code=COMM_ERR_PORT_ERROR;
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
  timer_frame_1ms=(7UL * 11000UL) /(2UL*ulBaudRate ); 
 }
 if(xcomm_port_timer_init(timer_frame_1ms)==COMM_FALSE)
 err_code=COMM_ERR_PORT_ERROR;
   
 return err_code;
}


void comm_serial_start(void)
{
 COMM_ENTER_CRITICAL_SECTION();
 comm_recv_state=COMM_RECV_STATE_INIT;
 vcomm_port_serial_enable(COMM_TRUE,COMM_FALSE);/*使能串口接收，禁止串口输出*/
 vcomm_port_timer_enable();
 COMM_EXIT_CRITICAL_SECTION();
}

void comm_serial_stop(void)
{
 COMM_ENTER_CRITICAL_SECTION();
 vcomm_port_serial_enable(COMM_FALSE,COMM_FALSE);/*使能串口接收，禁止串口输出*/
 vcomm_port_timer_disable();
 COMM_EXIT_CRITICAL_SECTION();
}


/*
 *处理接收到的一帧数据
 */
comm_err_code_t comm_serial_recv_fsm(uint8_t *ptr_addr,uint8_t **ptr_fsm,uint8_t *ptr_len)
{
    comm_bool_t        fsm_received = COMM_BOOL_FALSE;
    comm_err_code_t    err_code = COMM_ERR_NO_ERR;

    COMMU_ENTER_CRITICAL_SECTION();
    /*长度校验*/
    if((comm_recv_cnt <= COMM_RECV_MAX )&& (recv_buff_pos >= COMM_RECV_MIN))
    {
        /* 保存地址*/
        *ptr_addr = comm_serial_buff[COMM_ADDR_OFFSET];
        /* 计算接收的除去地址后的数据长度 */
        *ptr_len =(uint8_t)(comm_recv_cnt-COMM_ADDR_SIZE);

        /*把除去地址后的数据返回给调用者*/
        *ptr_fsm = (uint8_t*)&comm_serial_buff[COMM_CMD_OFFSET];
        fsm_received = COMM_BOOL_TRUE;
    }
    else
    {
       err_code = COMM_ERR_IO_ERR;
    }

   COMMU_EXIT_CRITICAL_SECTION();
   return err_code;
}
/*
 *处理要发送的一帧数据
 */
comm_err_code_t comm_serial_send_fsm(uint8_t slave_addr,uint8_t *ptr_fsm,uint8_t len)
{
 comm_err_code_t err_code=COMMU_ERR_NO_ERR; 

  COMMU_ENTER_CRITICAL_SECTION();
  if(comm_recv_state==COMM_RECV_STATE_IDLE)
  {
  /*第一个字节是地址*/
   ptr_send=ptr_fsm-COMM_ADDR_SIZE;
   ptr_send[COMM_ADDR_OFFSET]=slave_addr;
   comm_send_cnt=1;
   comm_send+=len;
   comm_send_state=COMM_SEND_STATE_XMIT;
   vcomm_port_enable_serial(COMM_BOOL_FALSE,COMM_BOOL_TRUE);  
  }
  else
  {
   err_code=COMMU_ERR_IO;
  }
  COMMU_EXIT_CRITICAL_SECTION();
  return err_code;
}
comm_err_code_t comm_serial_recv_byte(void)
{
  uint8_t recv_byte;
  xcomm_serial_get_byte(&recv_byte);
  switch(comm_recv_state)
    {
    /*如果在接收时还没有开始，就重置定时器*/
    case COMM_RECV_STATE_INIT:
        vcomm_port_timer_enable();
        break;
    /*在错误的状态下，重置定时器*/
    case COMM_RECV_STATE_ERROR:
        vcomm_port_timer_enable();
        break;
    /*在空闲状态下第一次接收数据，开启定时器，置状态为正在接收数据*/
    case COMM_RECV_STATE_IDLE:
        comm_recv_cnt = 0;
        comm_serial_buff[comm_recv_cnt++] = recv_byte;
        comm_recv_state = COMM_RECV_STATE_RECV;
        /* Enable t3.5 timers. */
        vcomm_port_timer_enable();
        break;

        /* We are currently receiving a frame. Reset the timer after
         * every character received. If more than the maximum possible
         * number of bytes in a modbus frame is received the frame is
         * ignored.
         */
    case COMM_RECV_STATE_RECV:
        if( usRcvBufferPos < MB_SER_PDU_SIZE_MAX )
        {
            ucRTUBuf[usRcvBufferPos++] = ucByte;
        }
        else
        {
            eRcvState = STATE_RX_ERROR;
        }
        vMBPortTimersEnable(  );
        break;
    }
    return xTaskNeedSwitch;
}
}



uint8_t port_serial_init()
{
port_
port_comm_timer_init(); 
  
return FORTUNA_TRUE;
}




