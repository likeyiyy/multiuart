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
#define     MAX_BUFSIZ      4096

int raw_uart_recv_handler(uart_dev_t * dev)
{
    int result = -1;
    int cur_index = dev->cur_index;
    for(int i = 0; i < 10; i++)
    {
        result = read(dev->fd, 
                      dev->buffer + cur_index, 
                      MAX_BUFSIZ - cur_index);
        LOG_DEBUG("[RAW_UART_RECV]: this %d times read %d bytes",i,result);
        cur_index += result;
    }
    dev->cur_index = cur_index;
    return 0;
}

int raw_uart_timeout_handler(uart_dev_t * dev)
{
    message_t * message = make_message(dev->name, dev->buffer,dev->cur_index);
    LOG_DEBUG("xxxxxxxxxx: start");
    view_message(message);
    LOG_DEBUG("XXXXXXXXXX: end");
    uart_recv_enqueue(dev,message);
    pthread_mutex_unlock(&dev->serial_lock);
    dev->cur_index = 0;
    return 0;
}

int raw_socket_recv_handler(uart_dev_t * dev,
                            message_t * message,
                            message_t ** fit)
{
    int result = queue_dequeue(dev->recv_queue,(void **)fit);
    if(result < 0)
    {
        *fit = NULL;
        return -1;
    }
    return 0;
}
