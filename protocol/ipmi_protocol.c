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
#include "multiuart_common.h"
#define     MsgBufferSize   2048

#define ENABLE              1
#define DISABLE             0

static inline int compare_message(message_t * message, recv_header_t * recv_header)
{
    common_header_t * common_header = (common_header_t *)message->data;
    LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    LOG_DEBUG("slave_addr  : %02x r.slave_addr  : %02x",common_header->slaver_addr,recv_header->slaver_addr);
    LOG_DEBUG("master_addr : %02x r.master_addr : %02x",common_header->master_addr,recv_header->master_addr);
    LOG_DEBUG("function    : %02x r.function    : %02x",common_header->netfn_rslun,recv_header->function);
    LOG_DEBUG("seq         : %02x r.seq         : %02x",common_header->rqseq_rqlun & 0x80,recv_header->seq);
    LOG_DEBUG("command     : %02x ",common_header->command);
    LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    if((common_header->slaver_addr  == recv_header->slaver_addr)&&
       (common_header->master_addr == recv_header->master_addr)&&
       (common_header->netfn_rslun == recv_header->function) && 
      ((common_header->rqseq_rqlun & 0x80) == recv_header->seq))
    {
        LOG_DEBUG("yes 目前为止一切都好");
        for(int i = 0; i < recv_header->command->nums; i++)
        {
            if(common_header->command == recv_header->command->command[i])
            {
                LOG_DEBUG("yes we found a one");
                return 0;
            }
        }
    }
    LOG_DEBUG("finally, we not found it");
    return 1;
}

void clear_buffer(uart_dev_t * dev, int start, int end)
{
    for(int i = start; i != (end + 1); i = (i + 1) % MsgBufferSize)
    {
        dev->buffer[i] = 0;
    }
}
int _ipmi_uart_parser(uart_dev_t * dev, int * start )
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
            uint8_t * buffer = malloc(length);
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
            LOG_DEBUG("[IPMI_UART_RECV]: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
            view_message(message);
            LOG_DEBUG("[IPMI_UART_RECV]: ************************************");
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
int ipmi_uart_recv_handler(uart_dev_t * dev)
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
    while(!_ipmi_uart_parser(dev,&start))
    {
        continue;
    }
    return 0;
}
int ipmi_socket_recv_handler(uart_dev_t * dev, 
                            message_t * message,
                            message_t ** fit)
{
    uint8_t * recv_buf = message->data;
    int n = message->length;
    recv_header_t recv_header_body = *(recv_header_t *)recv_buf;
    recv_header_t * recv_header = &recv_header_body;
    command_t * command = malloc(sizeof(command_t));
    VERIFY(command,"[MULTIUART]: command malloc failed");
    command->nums   = *(char *)(recv_buf + (sizeof(recv_header_t) - sizeof(command_t *)));
    command->command = (uint8_t *)(recv_buf + (sizeof(recv_header_t) - sizeof(command_t *) + 1));
    recv_header->command = command;
    LOG_DEBUG("####################### recv Handler start #########################");
    LOG_DEBUG("Recv header length : %d",n);
    LOG_DEBUG("[Handler]: slaver_addr:%02x master_addr:%02x function:%02x",
          recv_header->slaver_addr,
          recv_header->master_addr,
          recv_header->function);
    LOG_DEBUG("command->nums : %d",command->nums);
    for(int i = 0; i < command->nums; i++)
    {
        LOG_DEBUG("support command: %02x",command->command[i]);
    }
    int qlen = queue_size(dev->recv_queue);
    LOG_DEBUG("queue_length:%d",qlen);
    LOG_DEBUG("####################### recv Handler end #########################");
    message_t * temp = NULL;
    for(int i = 0; i < qlen; i++)
    {
        queue_dequeue(dev->recv_queue, (void **)&temp);
        if(!compare_message(temp,recv_header))
        {
            *fit = temp;
            return 0;
        }
        else
        {
            queue_enqueue(dev->recv_queue, temp);
        }
    }
    *fit = NULL;
    return -1;
}
int ipmi_uart_timeout_handler(uart_dev_t * dev)
{
    return 0;
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
    while(!_ipmi_uart_parser(dev,&start))
    {
        continue;
    }
}
#endif
