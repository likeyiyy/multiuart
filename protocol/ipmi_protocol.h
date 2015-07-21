/*************************************************************************
	> File Name: ipmi_protocol.h
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Mon 20 Jul 2015 12:50:18 PM CST
 ************************************************************************/

#ifndef _IPMI_PROTOCOL_H
#define _IPMI_PROTOCOL_H

#define ENABLE              1
#define DISABLE             0

#define DATA_LEN            16
#define TEMP_CMD            0x0A
#define FORWARD_CMD         0x0C
#define STATUS_IDLE         0
#define STATUS_START        1
#define STATUS_MASTER_ADDR  2
#define STATUS_FUNC         3
#define STATUS_CHECKSUM     4
#define STATUS_SLAVE_ADDR   5
#define STATUS_RQSEQ        6
#define STATUS_COMMAND      7
#define STATUS_COMPLETE     8
#define STATUS_DATA         9
#define STATUS_STOP         10

#define UART_START          0xA0
#define UART_STOP           0xA5

#define UART_MASTER_ADDR    0x51
#define UART_SLAVE_ADDR     0x93
#define UART_COMPLETE       0x00
#define BLOCK_SIZE          3

#define PSOC_ADDR    0x93
#ifdef TILEGX
#define FORWARD_ADDR 0xdd
#define MASTER_ADDR  0x51
#else
#define FORWARD_ADDR 0x51
#define MASTER_ADDR  0xdd
#endif
#define TEMP_CMD            0x0A
#define FORWARD_CMD         0x0C

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
    uint8_t slaver_addr;
    uint8_t master_addr;
    uint8_t function;
    uint8_t seq;
    command_t * command;
}__attribute__((__packed__)) recv_header_t;

typedef struct 
{
    uint8_t start;
    uint8_t slaver_addr;
    uint8_t netfn_rslun;
    uint8_t head_checksum;
    uint8_t master_addr;
    uint8_t rqseq_rqlun;
    uint8_t command;
}__attribute__((__packed__)) common_header_t;

int ipmi_uart_recv_handler(uart_dev_t * dev);
int ipmi_socket_recv_handler(uart_dev_t * dev, message_t * message,
                            message_t ** hit);

#endif
