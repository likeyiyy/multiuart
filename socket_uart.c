/*************************************************************************
	> File Name: uart_socket.c
	> Author: 
	> Mail: 
	> Created Time: Fri 23 Jan 2015 02:45:48 PM CST
 ************************************************************************/
#include "multiuart_common.h"
#include "config.h"
#include "protocol/ipmi_protocol.h"
#include "protocol/raw_protocol.h"
#include "protocol/proto_manager.h"
#define SERIALIZE_MASK      0x1
#define SERIALIZE_LOCK_MASK 0x2

typedef struct
{
    uart_dev_t * devs;
    int dev_nums;
    queue_t * send_queue;
    int configed;
}socket_context_t;

static socket_context_t socket_context_body;
static socket_context_t * context = &socket_context_body;

static int open_server_socket(const char * domain_file)
{
    int listen_fd;  
    int ret;  
    struct sockaddr_un srv_addr;  
    unlink(domain_file);  
    listen_fd = socket(PF_UNIX,SOCK_STREAM,0);  
    if(listen_fd < 0)  
    {  
        perror("cannot create communication socket");  
        return -1;  
    }    
    int on = 1;  
    if((setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))) < 0)  
    {  
        perror("setsockopt failed");  
        return -1;
    }  
    //set server addr_param  
    srv_addr.sun_family=AF_UNIX;  
    strncpy(srv_addr.sun_path,domain_file,sizeof(srv_addr.sun_path) - 1);  
    unlink(domain_file);  
    ret = bind(listen_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));  
    if(ret == -1)  
    {  
        perror("cannot bind server socket");  
        close(listen_fd);  
        unlink(domain_file);  
        return -1;  
    }  
    //listen sockfd   
    ret = listen(listen_fd,1);  
    if(ret == -1)  
    {  
        perror("cannot listen the client connect request");  
        close(listen_fd);  
        unlink(domain_file);  
        return -1;  
    }  
    return listen_fd;
}
static void * uart_send_worker(void * arg)
{
    message_t * message ;
    struct timeval now;
    while(1)
    {
        while(queue_dequeue(context->send_queue,(void **)&message) != 0)
        {
            usleep(1000);
            continue;
        }
        gettimeofday(&now,NULL);
        if((now.tv_sec - message->stamp.tv_sec) > 10)
        {
            free_message(message);
            continue;
        }
        LOG_DEBUG("[%s] data:%p, length:%d",
                  __func__,
                  message->data,
                  message->length);
        //此串口需要串口化
        uart_dev_t * dev = (uart_dev_t *)message->dev;

        if(dev->flags & SERIALIZE_MASK)
        {
            if(pthread_mutex_trylock(&dev->serial_lock) == 0)
            {
                raw_uart_send(dev->fd, message);
                free_message(message);
            }
            else
            {
                queue_enqueue(context->send_queue,message);
            }
        }
        else
        {
            raw_uart_send(dev->fd, message);
            free_message(message);
        }
        usleep(1000);
    }
    return NULL;
}

static inline int strchrtimes(char * buffer, int length, char chr)
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

static inline int is_invalid_message(char * buffer, int length)
{

    int result = strncmp(buffer,"multiuart",strlen("multiuart"));
    LOG_DEBUG("result:%d",result);
    if(result != 0)
    {
        return 1;
    }
    int chrtimes = strchrtimes(buffer,length,'#');
    LOG_DEBUG("chrtimes:%d",chrtimes);
    if(chrtimes < 3)
    {
        return 1;
    }
    return 0;
}

static inline void process_message_invalid_message(int client)
{
    write(client,"InvalidMessage",strlen("InvalidMessage"));
}

static inline void process_message_not_fit(int client)
{
    write(client,"NotfitMessage",strlen("NotfitMessage"));
}

static inline void process_message_fitted(int client, message_t * fit)
{
    uint8_t buffer[2048] = {0};
    int  length = 2048;
    serialized_message(fit, buffer, &length);
    write(client, buffer, length);
    free_message(fit);
}

static inline void process_message_succeed(int client)
{
    write(client,"SucceedMessage",strlen("SucceedMessage"));
}

static inline uart_dev_t * get_dev_by_name(char * name)
{
    for(int i = 0; i < context->dev_nums; i++)
    {
        if(!strcmp(context->devs[i].name, name))
        {
            return &context->devs[i];
        }
    }
    return NULL;
    
}

static inline void process_message_invalid_name(int client, char * name)
{
    char  buffer[128];
    sprintf(buffer,"InvalidName:%s",name);
    write(client,buffer,strlen(buffer) + 1);
}

void * socket_uart_send_manager(void * arg)
{
    int listen_fd = open_server_socket(UNIX_SOCKET_UART_SEND);
    int com_fd;  
    socklen_t len;  
    struct sockaddr_un clt_addr;  
    pthread_t real_send;
    pthread_create(&real_send,
                  NULL,
                  uart_send_worker,
                  NULL);
    while(1)
    {
        len = sizeof(clt_addr);  
        com_fd = accept(listen_fd,(struct sockaddr*)&clt_addr,&len);  
        if(com_fd < 0)  
        {  
            perror("cannot accept client connect request");  
            close(listen_fd);  
            unlink(UNIX_SOCKET_UART_SEND);  
            exit(-1);
        }  
        LOG_DEBUG("I accpet a message");
        int n = 0;
        uint8_t recv_buf[MAX_BUFSIZ];   
        if( (n = read(com_fd,recv_buf,sizeof(recv_buf))) < 0)
        {
            LOG_ERROR("[%s] Read Error\n",__func__);
            exit(-1);
        }
        LOG_DEBUG("I read a message: %s##########",recv_buf);
        if(is_invalid_message((char *)recv_buf, n))
        {
            LOG_ERROR("InvalidMessage");
            process_message_invalid_message(com_fd);
            close(com_fd);
        }
        else
        {
	        message_t * message = deserialized_message(recv_buf,n);
            view_message(message);
            uart_dev_t * dev = get_dev_by_name(message->name);
	        if(dev == NULL)
	        {
                LOG_ERROR("InvalidName");
	            process_message_invalid_name(com_fd, message->name);
	            close(com_fd);
	        }
	        else
	        {
                LOG_DEBUG("validMessage");
                message->dev = dev;
	            queue_enqueue(context->send_queue, message);
                process_message_succeed(com_fd);
                close(com_fd);
	        }
        }
    }
    return NULL;
}
static inline int _read_from_uart(uart_dev_t * dev)
{
    int result = read(dev->fd, &dev->buffer[dev->cur_index], MAX_BUFSIZ);
    if(result < 0)
    {
        LOG_ERROR("[MULTIUART]: read from uart error %s",dev->name);
        return -1;
    }
    return result;
}
static inline void * uart_recv_worker(void * arg)
{
    int result;
    fd_set readfds;
    int max_fd = 0;
    struct timeval timeout = {5,0};
    while(1)
    {
        FD_ZERO(&readfds);
        for(int i = 0; i < context->dev_nums; i++)
        {
            if(max_fd < context->devs[i].fd)
            {
                max_fd = context->devs[i].fd;
            }
            FD_SET(context->devs[i].fd, &readfds);
        }
        timeout.tv_sec  = 1000;
        timeout.tv_usec = 0;
        result = select(max_fd + 1,
                       &readfds,
                       NULL,
                       NULL,
                       &timeout);
        if(result < 0)
        {
		    LOG_ERROR("[MULTIUART]: Host monitor the read file error");
        }
        else if(result == 0)
        {
		    LOG_DEBUG("[MULTIUART]: Host read the packets timeout");
        }
        else
        {
            for(int i = 0; i < context->dev_nums; i++)
            {
                if(FD_ISSET(context->devs[i].fd, &readfds))
                {
                    uart_recv_handler * process_uart_recv = get_uart_recv_handler_by_protocol(context->devs[i].protocol);
                    process_uart_recv->func(&context->devs[i]);                    
                }
            }
        }
    }
    return NULL;
}
void * socket_uart_recv_manager(void * arg)
{
    int listen_fd = open_server_socket(UNIX_SOCKET_UART_RECV);
    pthread_t real_recv;
    int result = pthread_create(&real_recv,
                                NULL,
                                uart_recv_worker,
                                NULL);
    assert(result == 0);
    socklen_t len = -1;
    struct sockaddr_un clt_addr;  
    int com_fd = -1;
    while(1)
    {
        len = sizeof(clt_addr);  
        com_fd = accept(listen_fd,(struct sockaddr*)&clt_addr,&len);  
        if(com_fd < 0)  
        {  
            perror("cannot accept client connect request");  
            close(listen_fd);  
            unlink(UNIX_SOCKET_UART_SEND);  
            exit(-1);
        }  
        int n = 0;
        uint8_t recv_buf[MAX_BUFSIZ];   
        if( (n = read(com_fd,recv_buf,sizeof(recv_buf))) < 0)
        {
            LOG_ERROR("[%s] Read Error\n",__func__);
        }
        LOG_DEBUG("################%s,%d############\n",recv_buf,n);
        if(is_invalid_message((char *)recv_buf, n))
        {
            process_message_invalid_message(com_fd);
            close(com_fd);
        }
        else
        {
	        message_t * message = deserialized_message(recv_buf,n);
            view_message(message);
            uart_dev_t * dev = get_dev_by_name(message->name);
	        if(dev == NULL)
	        {
	            process_message_invalid_name(com_fd, message->name);
	            close(com_fd);
	        }
            else
            {
                struct timeval now;
                gettimeofday(&now,NULL);
                message_t * fit = NULL;
                socket_recv_handler * process_socket_recv = get_socket_recv_handler_by_protocol(dev->protocol);
                process_socket_recv->func(dev, message, &fit);
                if((fit == NULL))
                {
                    process_message_not_fit(com_fd);
                }
                else if((now.tv_sec - fit->stamp.tv_sec) > 10)
                {
                    free_message(fit);
                    process_message_not_fit(com_fd);
                }
                else
                {
                    process_message_fitted(com_fd, fit);
                }
                free_message(message);
                close(com_fd);
            }
        }
    }
    return NULL;
}
int socket_uart_init(config_t * config)
{
    VERIFY(context, "socket context is NULL");
    VERIFY(config,  "socket config is NULL");
    VERIFY(config->devs, "config devs is NULL");

    context->devs = malloc(sizeof(uart_dev_t) * config->dev_nums);
    context->dev_nums = config->dev_nums;
    context->send_queue = queue_init(8,TMC_QUEUE_SINGLE_RECEIVER);

    for(int i = 0; i < config->dev_nums; i++)
    {
        strcpy(context->devs[i].name,config->devs[i].name);
        context->devs[i].rbaud = config->devs[i].rbaud;
        strcpy(context->devs[i].protocol,config->devs[i].protocol);
        if(!strcmp(context->devs[i].protocol,"raw"))
        {
            context->devs[i].flags = 1;
        }
        pthread_mutex_init(&context->devs[i].serial_lock, NULL);
        context->devs[i].recv_queue = queue_init(8, TMC_QUEUE_SINGLE_RECEIVER);
        context->devs[i].cur_index = 0;
        VERIFY((context->devs[i].fd = init_uart_device(&context->devs[i])) > 0,"Not Such device");
        if(context->devs[i].fd < 0)
        {
            LOG_ERROR("%s open failed",context->devs[i].name);
        }
    }
    context->configed = 1;
    return 0;
}
