/*************************************************************************
	> File Name: raw_protocol.h
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Mon 20 Jul 2015 12:50:23 PM CST
 ************************************************************************/

#ifndef _RAW_PROTOCOL_H
#define _RAW_PROTOCOL_H

int raw_uart_recv_handler(uart_dev_t * dev);
int raw_socket_recv_handler(uart_dev_t * dev, message_t * message);
#endif
