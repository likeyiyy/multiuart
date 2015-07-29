/*************************************************************************
	> File Name: proto_manager.h
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Mon 20 Jul 2015 03:33:10 PM CST
 ************************************************************************/
#ifndef _PROTO_MANAGER_H
#define _PROTO_MANAGER_H
typedef int ( * _uart_recv_handler)(uart_dev_t * dev);
typedef struct _recvhandler
{
    char protocol[16];
    _uart_recv_handler func;
}uart_recv_handler;
typedef uart_recv_handler uart_timeout_handler;
typedef struct
{
    uart_recv_handler * handlers;
    int number;
}uart_handlers;
int register_uart_recv_handler(char * name, _uart_recv_handler handler);
uart_recv_handler * get_uart_recv_handler_by_protocol(char * protocol);

typedef int ( * _socket_recv_handler)(uart_dev_t * dev, 
                message_t * message, 
                message_t ** fit);
typedef struct 
{
    char protocol[16];
    _socket_recv_handler func;
}socket_recv_handler;

typedef struct
{
    socket_recv_handler * handlers;
    int number;
}socket_handlers;

int register_socket_recv_handler(char * name, _socket_recv_handler handler);
socket_recv_handler * get_socket_recv_handler_by_protocol(char * protocol);
#endif
