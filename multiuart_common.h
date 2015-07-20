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


#define  MAX_BUFSIZ  1024

#define PSOC_ADDR 0x93
#ifdef TILEGX
#define FORWARD_ADDR 0xdd
#define MASTER_ADDR  0x51
#else
#define FORWARD_ADDR 0x51
#define MASTER_ADDR  0xdd
#endif
#define TEMP_CMD            0x0A
#define FORWARD_CMD         0x0C

extern uint32_t request_size ;

#include "list.h"
#include "message.h"
#include "queue.h"

typedef struct 
{
    uint8_t nums;
    uint8_t * command;
}__attribute__((__packed__))command_t;

typedef struct 
{
    uint8_t index;
    uint8_t recv_slave_addr;
    uint8_t send_slave_addr;
    uint8_t master_addr;
    uint8_t send_function;
    uint8_t recv_function;
    command_t * command;
    uint8_t cur_command;
    uint8_t complete;
}uart_context_t;


typedef struct
{
    uint8_t index;
    message_t * message;
}__attribute__((__packed__)) packet_t;

typedef struct
{
    uint8_t index;
    uint8_t slaver_addr;
    uint8_t master_addr;
    uint8_t function;
    uint8_t seq;
    command_t * command;
}__attribute__((__packed__)) recv_header_t;

#define UNIX_SOCKET_UART_SEND  "/tmp/socket_uart_send.domain"
#define UNIX_SOCKET_UART_RECV  "/tmp/socket_uart_recv.domain"

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
typedef struct 
{
    uint8_t start;
    uint8_t slave_addr;
    uint8_t netfn_rslun;
    uint8_t head_checksum;
    uint8_t master_addr;
    uint8_t rqseq_rqlun;
    uint8_t command;
}__attribute__((__packed__)) common_header_t;


#include "raw_uart.h"
#include "socket_uart.h"

#endif
