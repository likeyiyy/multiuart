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

Handlers handlers_body = {NULL, 0};
Handlers * context = &handlers_body;

int register_recv_handler(char * name, recv_handler handler)
{
    context->handlers = realloc(context->handlers,
                                (context->number + 1) * sizeof(RecvHandler));
    VERIFY(context->handlers,
          "context handlers realloc failed");
    strcpy(context->handlers[context->number].protocol,name);
    context->handlers[context->number].func = handler;
    ++context->number;
    return 0;
}
RecvHandler * get_recv_handler_by_protocol(char * protocol)
{
    for(int i = 0; i < context->number; i++)
    {
        if(!strcmp(context->handlers[i].protocol, protocol))
        {
            return &context->handlers[i];
        }
    }
    LOG_NOTICE("[MULTIUART]: get_recv_handler_by_protocol failed, not support such protocol");
    return NULL;
}

