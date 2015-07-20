/*************************************************************************
	> File Name: proto_manager.h
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Mon 20 Jul 2015 03:33:10 PM CST
 ************************************************************************/

#ifndef _PROTO_MANAGER_H
#define _PROTO_MANAGER_H
typedef int ( * recv_handler)(uart_dev_t * dev);

typedef struct _RecvHandler
{
    char protocol[16];
    recv_handler func;
}RecvHandler;

typedef struct
{
    RecvHandler * handlers;
    int number;
}Handlers;
int register_recv_handler(char * name, recv_handler handler);
RecvHandler * get_recv_handler_by_protocol(char * protocol);
#endif
