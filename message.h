/*************************************************************************
	> File Name: message.h
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Fri 17 Jul 2015 10:17:08 AM CST
 ************************************************************************/

#ifndef _MESSAGE_H
#define _MESSAGE_H

#define HEADER_LEN 3
#include <stdint.h>
typedef struct 
{
    char name[16];
    char cmd[16];
    uint16_t length;
    uint8_t * data;
    struct timeval stamp;
}message_t;

message_t * make_message(int index, uint8_t * recv_buf, int length);
message_t * deserialized_message(uint8_t * recv_buf, int length);
int serialized_message(message_t * message, uint8_t * buffer, int * length);
void free_message(message_t * message);

#endif
