#ifndef  __COMM_PORT_SERIAL_H__
#define  __COMM_PORT_SERIAL_H__


#define  COMM_BAUDRATE                               115200UL

comm_status_t xcomm_port_serial_init();
void xcomm_port_serial_enable(app_bool_t rx_bool,app_bool_t tx_bool);
/*串口接收一个字节*/
void xcomm_port_serial_send_byte(uint8_t send_byte);
/*串口发送一个字节*/
void xcomm_port_serial_get_byte(uint8_t *ptr_byte);

/*串口中断处理*/
void xcomm_port_serial_isr(void);




#endif