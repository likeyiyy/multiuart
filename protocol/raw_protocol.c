/*************************************************************************
	> File Name: raw_protocol.c
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Mon 20 Jul 2015 12:49:06 PM CST
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "llog.h"
#include "raw_uart.h"
#include "raw_protocol.h"
#define     MAX_BUFSIZ      1024

int raw_uart_recv_handler(uart_dev_t * dev)
{
    int result = read(dev->fd, 
                     dev->buffer, 
                     MAX_BUFSIZ);
    if(result < 0)
    {
        LOG_ERROR("[MULTIUART]: read from uart error %s",dev->name);
        return -1;
    }
    message_t * message = make_message(dev->name, dev->buffer, result);
    uart_recv_enqueue(dev,message);
    pthread_mutex_unlock(&dev->serial_lock);
    return 0;
}

int raw_socket_recv_handler(uart_dev_t * dev,
                            message_t * message,
                            message_t ** fit)
{
    
    return 0;
}
