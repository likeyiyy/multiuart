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
    LOG_NOTICE("Title:  %s",message->title);
    LOG_NOTICE("Name:   %s",message->name);
    LOG_NOTICE("Length: %d",message->length);
    LOG_NOTICE("Stamp:  %lu.%lu",message->stamp.tv_sec, message->stamp.tv_usec);
    char temp[128];
    int counter = 0;
    for(int i = 0; i < message->length; i++)
    {
        if(i && i % 16 == 0)
        {
            LOG_NOTICE("%s",temp);
            counter = 0;
        }
        sprintf(temp + counter, "%02x ",message->data[i]);
        counter += 3;
    }
    LOG_NOTICE("%s",temp);
}
message_t * deserialized_message(uint8_t * recv_buf, int length)
{
    int chrtimes = strchrtimes(recv_buf,length,'#');
    char delims[] = "#";
    char * length_str = NULL;
    if(chrtimes >= 3)
    {
        message_t * message = malloc(sizeof(message_t));
        VERIFY(message, "message alloc failed");
        strcpy(message->title, strtok((char *)recv_buf,delims));
        strcpy(message->name,  strtok(NULL,delims));
        length_str = strtok(NULL,delims);
        message->length = atoi(length_str);
        int header_len = 3 + strlen(message->title) + strlen(message->name) + strlen(length_str);
        int after = length - header_len;
        message->data = malloc(length - header_len);
        memcpy(message->data, recv_buf + header_len, after);
        gettimeofday(&message->stamp,NULL);
        return message;
    }
    return NULL;
}
int serialized_message(message_t * message, uint8_t * buffer, int * length)
{
    char temp[64];
    sprintf(temp,"%s#%s#%d#",message->title,
                             message->name,
                             message->length);
    int temp_len = strlen(temp);
    VERIFY((*length >= temp_len + message->length), 
           "length must bigger than message length and header");
    memcpy(buffer,temp,temp_len);
    memcpy(buffer + temp_len, message->data, message->length);
    *length = temp_len + message->length;
    return 0;
}
void free_message(message_t * message)
{
    free(message->data);
    free(message);
}

#ifdef MESSAGE_DEBUG
int main( int argc, char ** argv )
{
    llog_init(LL_NOTICE, stdout);
    char buffer[1024];
    char data[] = "hello world I am very happy with you now, and soon, but hahah eryi.";
    sLOG_NOTICE(buffer,"multiuart#/dev/ttyUSB2#%d#%s",strlen(data),data);
    int length    = strlen(buffer);
    LOG_NOTICE("%s\n",buffer);
    message_t * message =  deserialized_message(buffer, length);
    view_message(message);
    char buffer2[1024];
    serialized_message(message, buffer2, &length);
    LOG_NOTICE("%s:%d\n",buffer2,length);
}
#endif
