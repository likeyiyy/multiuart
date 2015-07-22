/*************************************************************************
	> File Name: multiuart_common.h
	> Author: 
	> Mail: 
	> Created Time: Fri 23 Jan 2015 04:11:10 PM CST
 ************************************************************************/

#ifndef _MULTIUART_COMMON_H
#define _MULTIUART_COMMON_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/time.h>  
#include <sys/un.h> 

#include "llog.h"



#define MAX_BUFSIZ 2048

extern uint32_t request_size ;

#include "list.h"
#include "message.h"
#include "queue.h"


#define UNIX_SOCKET_UART_SEND  "/tmp/socket_uart_send.domain"
#define UNIX_SOCKET_UART_RECV  "/tmp/socket_uart_recv.domain"

#if 0
static inline void print_buf(uint8_t * message,int length)
{
    for(int i = 0; i < length; i++)
    {
        if(i && i % 8 == 0)
        {
            LOG_DEBUG("\n");
        }
        LOG_DEBUG("0x%02x ",message[i]);
    }
    LOG_DEBUG("\n");

}
#endif


#include "raw_uart.h"
#include "socket_uart.h"

#endif
