#ifndef  __AT_CMD_PORT_SERIAL_H__
#define  __AT_CMD_PORT_SERIAL_H__



#define  AT_CMD_PORT_SERIAL_BAUDRATE         115200



void at_cmd_port_serial_init();
void at_cmd_port_serial_enable(at_cmd_bool_t rx_bool,at_cmd_bool_t tx_bool);
/*串口接收一个字节*/
void at_cmd_port_serial_send_byte(uint8_t send_byte);
/*串口发送一个字节*/
void at_cmd_port_serial_get_byte(uint8_t *ptr_byte);

/*串口中断处理*/
void at_cmd_port_serial_isr(void);




#endif