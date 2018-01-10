#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "usart.h"
#include "host_protocol.h"
#include "host_comm_task.h"
#include "comm_port_timer.h"
#define APP_LOG_MODULE_NAME   "[port_timer]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

static void xcomm_port_serial_timer_expired(void const * argument);
osTimerId host_comm_timer_id;
uint16_t timeout;

comm_status_t xcomm_port_serial_timer_init(uint16_t time)
{
 osTimerDef(host_comm_timer,xcomm_port_serial_timer_expired);
 host_comm_timer_id=osTimerCreate(osTimer(host_comm_timer),osTimerOnce,0);
 APP_ASSERT(host_comm_timer_id);
 timeout=time;
 return COMM_OK;
}
void xcomm_port_serial_timer_start()
{
 osTimerStart(host_comm_timer_id,timeout);
}
void xcomm_port_serial_timer_stop()
{
 osTimerStop(host_comm_timer_id);
}

static void xcomm_port_serial_timer_expired(void const * argument)
{
 void comm_fsm_timer_expired();
 comm_fsm_timer_expired();
}
