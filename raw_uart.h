#ifndef _RAW_PSOC_H
#define _RAW_PSOC_H

#include "message.h"
#include "queue.h"

typedef struct _uart_dev
{
    char name[16];
    int  rbaud;
    int  fd;
    char protocol[16];
    int  flags;
    pthread_mutex_t serial_lock;
    queue_t * recv_queue;
    unsigned char buffer[3000];
    int cur_index;
}uart_dev_t;

int raw_uart_send(int fd, message_t * message);
int init_uart_device(uart_dev_t * device);
void uart_recv_enqueue(uart_dev_t * device, message_t * message);

#endif
