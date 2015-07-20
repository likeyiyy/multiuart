/*************************************************************************
	> File Name: uart_deamon.c
	> Author: likeyi
	> Mail: likeyi@sina.com 
	> Created Time: Wed 28 Jan 2015 03:47:57 PM CST
 ************************************************************************/

#include "multiuart_common.h"
int main(int argc, char ** argv)
{
    int dev_nums = 0;
    char ** devs_name = NULL;
    for(int i = 1; i < argc; i++)
    {
        char * arg = argv[i];

        if(!strcmp(arg, "--dev_nums") && i + 1 < argc)
        {
            dev_nums = atoi(argv[i++]);
        }
    }
    for(int i = 1; i < argc; i++)
    {
        char * arg = argv[i];
        if(!strcmp(arg, "--dev") && i + 1 < argc)
        {
            devs_name = malloc(dev_nums * sizeof(char *));
            for(int j = 0; j < dev_nums; j++)
            {
                devs_name[j] = malloc(strlen(argv[++i]) + 1);
                strcpy(devs_name[j],argv[i]);
            }
        }
        else if(!strcmp(arg, "--dev_nums") && i + 1 < argc)
        {
            dev_nums = atoi(argv[++i]);
        }
        else
        {
            printf("[%s] Unsupport argument:%s\n",argv[0],arg);
            exit(-1);
        }
    }
    for(int i = 0; i < dev_nums; i++)
    {
        printf("device%d : %s\n",i,devs_name[i]);
    }
    
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
