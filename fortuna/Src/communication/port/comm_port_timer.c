#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "usart.h"
#include "comm_protocol.h"
#include "host_comm_task.h"
#include "comm_port_timer.h"
#define APP_LOG_MODULE_NAME   "[port_timer]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_ERROR    
#include "app_log.h"
#include "app_error.h"

static void xcomm_port_serial_timer_expired(void const * argument);
osTimerId host_comm_timer_id;


comm_status_t xcomm_port_serial_timer_init()
{
 osTimerDef(host_comm_timer,xcomm_port_serial_timer_expired);
 host_comm_timer_id=osTimerCreate(osTimer(host_comm_timer),osTimerOnce,0);
 APP_ASSERT(host_comm_timer_id);
 return COMM_OK;
}
void xcomm_port_serial_timer_start()
{
 APP_LOG_DEBUG("HOST timer.\r\n");
 osTimerStart(host_comm_timer_id,COMM_PORT_TIMER_T35_TIMEOUT);
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
