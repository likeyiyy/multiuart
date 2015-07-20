/*************************************************************************
	> File Name: uart_deamon.c
	> Author: likeyi
	> Mail: likeyi@sina.com 
	> Created Time: Wed 28 Jan 2015 03:47:57 PM CST
 ************************************************************************/

#include "multiuart_common.h"
#include "config.h"
#include "raw_uart.h"
#include "protocol/ipmi_protocol.h"
#include "protocol/raw_protocol.h"
#include "protocol/proto_manager.h"
int main(int argc, char ** argv)
{
    char config_file [256] = { 0 };
    int config_flag = 0;
    config_t config;
    for(int i = 1; i < argc; i++)
    {
        char * arg = argv[i];
        if((!strcmp(arg, "-c") || !strcmp(arg,"--config")) && i + 1 < argc)
        {
            config_flag = 1;
            strcpy(config_file, argv[++i]);
        }
        else
        {
            printf("[%s] Unsupport argument:%s\n",argv[0],arg);
            exit(-1);
        }
    }
    if(config_flag == 1)
    {
        read_config_file(config_file, &config);
    }
    else
    {
        LOG_ERROR("[MULTIUART]: You must specify the configuration file");
    }

    register_recv_handler("ipmi",ipmi_recv_handler);
    register_recv_handler("raw", raw_recv_handler);
    
    
    pthread_t send_deamon,recv_deamon;
    pthread_create(&send_deamon,
            NULL,
            socket_uart_send_manager,
            NULL);

    pthread_create(&recv_deamon,
            NULL,
            socket_uart_recv_manager,
            NULL);

    while(1)
    {
        sleep(1);
        fflush(stdout);
    }
    return 0;
}
