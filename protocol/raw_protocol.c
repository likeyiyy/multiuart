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
    int cur_index = 0;
    for(int i = 0; i < 50; i++)
    {
        result = read(dev->fd, 
                      dev->buffer + cur_index, 
                      MAX_BUFSIZ);
        LOG_DEBUG("[RAW_UART_RECV]: this %d times read %d bytes",i,result);
        cur_index += result;
        usleep(10*1000);
    }
    message_t * message = make_message(dev->name, dev->buffer, cur_index);
    uart_recv_enqueue(dev,message);
    pthread_mutex_unlock(&dev->serial_lock);
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
