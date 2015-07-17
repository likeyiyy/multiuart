/*************************************************************************
	> File Name: message.c
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Fri 17 Jul 2015 10:02:18 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "llog.h"
#include "message.h"

message_t * make_message(int index, uint8_t * recv_buf, int length)
{
    message_t * message = malloc(sizeof(message_t));
    VERIFY(message, "message alloc failed");
    message->length = length;
    message->data   = malloc(length);
    memcpy(message->data, recv_buf, length);
    gettimeofday(&message->stamp,NULL);
    return message;
}
message_t * deserialized_message(uint8_t * recv_buf, int length)
{
    message_t * message = malloc(sizeof(message_t));
    VERIFY(message, "message alloc failed");




    message->length = *(uint16_t*)((uint8_t *)recv_buf + 1);
    VERIFY(message->length != length - HEADER_LEN, 
            "[MULTIUART] message.length should be equal length -3");
    message->data = malloc(length - HEADER_LEN);
    memcpy(message->data, recv_buf + HEADER_LEN, length - HEADER_LEN);
    gettimeofday(&message->stamp,NULL);
    return message;
}
int serialized_message(message_t * message, uint8_t * buffer, int * length)
{

    /* payload length */
    VERIFY(message->length > *length - HEADER_LEN, 
            "[MULTIUART] message.length should be equal length -3");
    *(uint16_t *)(buffer + 1) = message->length;
    memcpy(buffer + HEADER_LEN, message->data, *length - HEADER_LEN);
    *length = message->length + 3;
}
void free_message(message_t * message)
{
    free(message->data);
    free(message);
}

#ifdef MESSAGE_DEBUG
int main( int argc, char ** argv )
{
    
}
#endif
