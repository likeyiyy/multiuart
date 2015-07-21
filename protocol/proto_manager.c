/*************************************************************************
	> File Name: proto_manager.c
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Mon 20 Jul 2015 03:33:06 PM CST
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "llog.h"
#include "raw_uart.h"
#include "ipmi_protocol.h"
#include "proto_manager.h"
uart_handlers uart_handlers_body = {NULL, 0};
uart_handlers * uart_context = &uart_handlers_body;
int register_uart_recv_handler(char * name, _uart_recv_handler handler)
{
    uart_context->handlers = realloc(uart_context->handlers,
                                (uart_context->number + 1) * sizeof(uart_recv_handler));
    VERIFY(uart_context->handlers,
          "uart_context handlers realloc failed");
    strcpy(uart_context->handlers[uart_context->number].protocol,name);
    uart_context->handlers[uart_context->number].func = handler;
    ++uart_context->number;
    return 0;
}
uart_recv_handler * get_uart_recv_handler_by_protocol(char * protocol)
{
    for(int i = 0; i < uart_context->number; i++)
    {
        if(!strcmp(uart_context->handlers[i].protocol, protocol))
        {
            return &uart_context->handlers[i];
        }
    }
    LOG_NOTICE("[MULTIUART]: get_recv_handler_by_protocol failed, not support such protocol");
    return NULL;
}

socket_handlers socket_handlers_body = {NULL, 0};
socket_handlers * socket_context = &socket_handlers_body;
int register_socket_recv_handler(char * name, _socket_recv_handler handler)
{
    socket_context->handlers = realloc(socket_context->handlers,
                                        (socket_context->number + 1) * 
                                        sizeof(socket_recv_handler));
    VERIFY(socket_context->handlers,
          "socket_context handlers realloc failed");
    strcpy(socket_context->handlers[socket_context->number].protocol,name);
    socket_context->handlers[socket_context->number].func = handler;
    ++socket_context->number;
    return 0;
}
socket_recv_handler * get_socket_recv_handler_by_protocol(char * protocol)
{
    for(int i = 0; i < socket_context->number; i++)
    {
        if(!strcmp(socket_context->handlers[i].protocol, protocol))
        {
            return &socket_context->handlers[i];
        }
    }
    LOG_NOTICE("[MULTIsocket]: get_socket_recv_handler_by_protocol failed, not support such protocol");
    return NULL;
}
