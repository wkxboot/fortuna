#ifndef  __COMM_PORT_SERIAL_H__
#define  __COMM_PORT_SERIAL_H__
#include "fortuna_common.h"

#define  SERIAL_BUFF_MAX          64


comm_status_t xcomm_port_serial_init(uint8_t port,uint32_t baudrate,uint8_t databits);
void xcomm_port_serial_enable(fortuna_bool_t rx_bool,fortuna_bool_t tx_bool);
/*获取接收一帧串口数据的地址和长度*/
void xcomm_port_serial_recv(uint8_t **ptr_buff,uint8_t *ptr_recv_len);
/*发送一帧串口数据*/
void xcomm_port_serial_send(uint8_t *ptr_buff,uint8_t send_len);






#endif