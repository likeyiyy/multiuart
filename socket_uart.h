/*************************************************************************
	> File Name: uart_socket.h
	> Author: 
	> Mail: 
	> Created Time: Fri 23 Jan 2015 02:45:51 PM CST
 ************************************************************************/

#ifndef _UART_SOCKET_H
#define _UART_SOCKET_H


int    socket_uart_init();
void * socket_uart_send_manager(void * arg);
void * socket_uart_recv_manager(void * arg);
#endif
