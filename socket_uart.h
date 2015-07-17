/*************************************************************************
	> File Name: uart_socket.h
	> Author: 
	> Mail: 
	> Created Time: Fri 23 Jan 2015 02:45:51 PM CST
 ************************************************************************/

#ifndef _UART_SOCKET_H
#define _UART_SOCKET_H
int    socket_uart_init();
void * UartSendManager(void * arg);
void * UartRecvManager(void * arg);
#endif
