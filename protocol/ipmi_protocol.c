/*************************************************************************
	> File Name: ipmi_protocol.c
	> Author: li.ky
	> Mail: likeyiyy@sina.com
	> Created Time: Mon 20 Jul 2015 12:49:46 PM CST
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "llog.h"
#include "raw_uart.h"
#include "ipmi_protocol.h"
#define     MAX_BUFSIZ      1024
#define     MsgBufferSize   2048

#define ENABLE              1
#define DISABLE             0

void clear_buffer(uart_dev_t * dev, int start, int end)
{
    for(int i = start; i != (end + 1); i = (i + 1) % MsgBufferSize)
    {
        dev->buffer[i] = 0;
    }
}
int _ipmi_parser(uart_dev_t * dev, int * start )
{
    uint8_t curchar;
    int start_index = 0;
    int start_mark  = 0;
    int end_mark    = 0;
    int end_index   = 0;
    
    for(int i = *start; i < MsgBufferSize; i++)
    {
        curchar = dev->buffer[i];
        if(curchar == UART_START)
        {
            start_mark  = 1;
            start_index = i;
            break;
        }
    }
    if(start_mark)
    {
        for(int i = start_index + 1; i != start_index; i = (i + 1) % MsgBufferSize)
        {
	        curchar = dev->buffer[i];
	        if(curchar == UART_STOP)
	        {
	            end_mark  = 1;
	            end_index = i;
                break;
	        }
        }
        if(end_mark)
        {

            int length = (end_index + MsgBufferSize - start_index + 1) % MsgBufferSize;
            * start = end_index;
            char * buffer = malloc(length);
            VERIFY(buffer, "[%s] ipmi malloc buffer error",dev->name);
            int counter = 0;
            for(int i = start_index; i != (end_index + 1); i = (i + 1) % MsgBufferSize)
            {
                if((dev->buffer[i] == 0xAA))
                {
                    i = (i + 1) % MsgBufferSize;
                    buffer[counter++] = dev->buffer[i] - 0x10;
                }
                else
                {
                    buffer[counter++] = dev->buffer[i];
                }
            }
            VERIFY((counter <= length), "[%s] ipmi counter and length should be same", dev->name);
            message_t * message = make_message(dev->name, buffer, counter);
            //view_message(message);
            uart_recv_enqueue(dev,message);
            clear_buffer(dev, start_index, end_index);
            free(buffer);
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

}
int ipmi_recv_handler(uart_dev_t * dev)
{
    int result = read(dev->fd, 
                     &dev->buffer[dev->cur_index], 
                     MAX_BUFSIZ);
    if(result < 0)
    {
        LOG_ERROR("[MULTIUART]: read from uart error %s",dev->name);
        return -1;
    }

    dev->cur_index += result;
    if(dev->cur_index >= MsgBufferSize)
    {
        for(int i = 0; i < dev->cur_index - MsgBufferSize; i++)
        {
            dev->buffer[i] = dev->buffer[MsgBufferSize + i];
        }
        dev->cur_index -= MsgBufferSize;
    }
    int start = 0;
    while(!_ipmi_parser(dev,&start))
    {
        continue;
    }
}

#ifdef IPMI_PROTOCOL
int main( int argc, char ** argv )
{
    llog_init(LL_DEBUG,stdout);
    uart_dev_t * dev = malloc(sizeof(uart_dev_t));
    strcpy(dev->name,  "/dev/ttyUSB2");
    strcpy(dev->buffer,"a1234567ldi7654321dfiiiiiiiiiiifs"
                        "ldfjalkjfalfjldajfldfja;lfja;fjkal;fja;fjka;fda;;asf"
                        "jlafjalffasldfhafhafhowfhlsdafhlkasfhsdlkafhaklfhalfh"
                        "asdklfa;fklafldafklahlfdhaflahlfldkafhsdlkfhadfklahfk"
                        "lakfakfhkafalflkaldfa;ff;asdfkl");

    dev->buffer[8] = UART_STOP;

    dev->buffer[10] = UART_START;
    dev->buffer[13] = 0xaa;
    dev->buffer[14] = 0xb5;
    dev->buffer[15] = 0xaa;
    dev->buffer[16] = 0xb0;
    dev->buffer[38] = UART_STOP;

    dev->buffer[40] = UART_START;
    dev->buffer[58] = UART_STOP;

    dev->buffer[1890] = UART_START;
    dev->recv_queue = queue_init(8,0);
    int start = 0;
    while(!_ipmi_parser(dev,&start))
    {
        continue;
    }
}
#endif
