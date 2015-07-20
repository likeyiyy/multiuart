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

#endif
