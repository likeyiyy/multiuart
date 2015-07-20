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

message_t * make_message(char * dev_name, uint8_t * recv_buf, int length)
{
    message_t * message = malloc(sizeof(message_t));
    VERIFY(message, "message alloc failed");
    strcpy(message->title, "multiuart");
    strcpy(message->name, dev_name);
    message->length = length;
    message->data   = malloc(length);
    memcpy(message->data, recv_buf, length);
    gettimeofday(&message->stamp,NULL);
    return message;
}
static inline int strchrtimes(uint8_t * buffer, int length, uint8_t chr)
{
    int counter = 0;
    for(int i = 0; i < length; i++)
    {
        if(buffer[i] == chr)
        {
            ++counter;
        }
    }
    return counter;
}
void view_message(message_t * message)
{
    printf("Title:  %s\n",message->title);
    printf("Name:   %s\n",message->name);
    printf("Length: %d\n",message->length);
    printf("Stamp:  %u.%u\n",message->stamp.tv_sec, message->stamp.tv_usec);
    for(int i = 0; i < message->length; i++)
    {
        if(i && i % 16 == 0)
        {
            printf("\n");
        }
        printf("%02x ",message->data[i]);
    }
    printf("\n");
}
message_t * deserialized_message(uint8_t * recv_buf, int length)
{
    int chrtimes = strchrtimes(recv_buf,length,'#');
    char delims[] = "#";
    char * title = NULL;
    char * length_str = NULL;
    if(chrtimes >= 3)
    {
        message_t * message = malloc(sizeof(message_t));
        VERIFY(message, "message alloc failed");
        strcpy(message->title, strtok(recv_buf,delims));
        strcpy(message->name,  strtok(NULL,delims));
        length_str = strtok(NULL,delims);
        message->length = atoi(length_str);
        int header_len = 3 + strlen(message->title) + strlen(message->name) + strlen(length_str);
        int after = length - header_len;
        message->data = malloc(length - header_len);
        memcpy(message->data, recv_buf + header_len, length - header_len);
        gettimeofday(&message->stamp,NULL);
        //view_message(message);
        return message;
    }
    return NULL;
}
#if 1
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
#endif

#ifdef MESSAGE_DEBUG
int main( int argc, char ** argv )
{
    llog_init(LL_NOTICE, stdout);
    char buffer[1024];
    char data[] = "hello world I am very happy with you now, and soon, but hahah eryi.";
    sprintf(buffer,"multiuart#/dev/ttyUSB2#%d#%s",strlen(data),data);
    int length    = strlen(buffer);
    deserialized_message(buffer, length);
    sprintf(buffer,"multiuart#/dev/ttyUSB2#%d#%s",strlen(data),data);
    printf("%s\n",buffer);
}
#endif
