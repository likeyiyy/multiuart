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
    listen_fd=socket(PF_UNIX,SOCK_STREAM,0);  
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
void * uart_send_worker(void * arg)
{
    char * data;
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
        LOG_DEBUG("[%s] data:%p, length:%d\n",
                  __func__,
                  message->data,
                  message->length);
        print_buf((uint8_t *)message->data, 
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

static int isInvalidMessage(char * buffer, int length)
{

    int result = strncmp(buffer,"multiuart",sizeof("multiuart"));
    if(result != 0)
    {
        return 1;
    }
    int chrtimes = strchrtimes(buffer,length,'#');
    if(chrtimes < 3)
    {
        return 1;
    }
    return 0;
}

static inline void ProcessMessageInvalidMessage(int client)
{
    write(client,"InvalidMessage",strlen("InvalidMessage") + 1);
}

static inline void ProcessMessageSucceed(int client)
{
    write(client,"SucceedMessage",strlen("SucceedMessage") + 1);
}

static uart_dev_t * get_dev_by_name(char * name)
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

static inline void ProcessMessageInvalidName(int client, char * name)
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
        int n = 0;
        char recv_buf[MAX_BUFSIZ];   
        if( (n = read(com_fd,recv_buf,sizeof(recv_buf))) < 0)
        {
            LOG_ERROR("[%s] Read Error\n",__func__);
        }
        if(isInvalidMessage(recv_buf, n))
        {
            ProcessMessageInvalidMessage(com_fd);
            close(com_fd);
        }
        else
        {
	        message_t * message = deserialized_message(recv_buf,n);
            uart_dev_t * dev = get_dev_by_name(message->name);
	        if(dev == NULL)
	        {
	            ProcessMessageInvalidName(com_fd, message->name);
	            close(com_fd);
	        }
	        else
	        {
                message->dev = dev;
	            queue_enqueue(context->send_queue, message);
                ProcessMessageSucceed(com_fd);
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
void * UartRecvWorker(void * arg)
{
    uint8_t recv_data = 0;
    int result;
    char buffer[MAX_BUFSIZ] = {0};
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
        timeout.tv_sec  = 10;
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
		    LOG_ERROR("[MULTIUART]: Host read the packets timeout");
        }
        else
        {
            for(int i = 0; i < context->dev_nums; i++)
            {
                if(FD_ISSET(context->devs[i].fd, &readfds))
                {
                    RecvHandler * ProcessRecv = get_recv_handler_by_protocol(context->devs[i].protocol);
                    ProcessRecv->func(&context->devs[i]);                    
                }
            }
        }
    }
    return NULL;
}
static inline int compare_blist(struct blist * node, recv_header_t * recv_header)
{
    message_t * message = (message_t *)node->data;
    common_header_t * common_header = (common_header_t *)message->data;
#if 1
    LOG_NOTICE(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    LOG_NOTICE("slave_addr  : %02x slave_addr  : %02x\n",common_header->slave_addr,recv_header->slaver_addr);
    LOG_NOTICE("master_addr : %02x master_addr : %02x\n",common_header->master_addr,recv_header->master_addr);
    LOG_NOTICE("function    : %02x function    : %02x\n",common_header->netfn_rslun,recv_header->function << 2);
    LOG_NOTICE("seq         : %02x seq         : %02x\n",common_header->rqseq_rqlun,recv_header->seq);
    LOG_NOTICE(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
#endif 
    if((common_header->slave_addr  == recv_header->slaver_addr)&&
       (common_header->master_addr == recv_header->master_addr)&&
       (common_header->netfn_rslun == recv_header->function << 2 ) && 
      ((common_header->rqseq_rqlun & 0x80) == recv_header->seq))
    {
        for(int i = 0; i < recv_header->command->nums; i++)
        {
            if(common_header->command == recv_header->command->command[i])
            {
                return 0;
            }
        }
    }
    return 1;
}
#if 0
void * handler_uart_recv(void * arg)
{
    pthread_detach(pthread_self());
    socket_context_t * context = (socket_context_t *)arg;
    socket_uart_t * so_uart = context->so_uart;
    int fd = context->fd;
    int n = 0;
    uint8_t recv_buf[MAX_BUFSIZ];   
    if( (n = read(fd,recv_buf,MAX_BUFSIZ)) < sizeof(recv_header_t) - sizeof(char *))
    {
        LOG_ERROR("[%s] Read Error\n",__func__);
        close(fd);
        return NULL;
    }
    recv_header_t recv_header_body = *(recv_header_t *)recv_buf;
    recv_header_t * recv_header = &recv_header_body;
    command_t * command = malloc(sizeof(command_t));
    assert(command);
    command->nums   = *(uint8_t *)(recv_buf + (sizeof(recv_header_t) - sizeof(command_t *)));
    command->command = (uint8_t *)(recv_buf + (sizeof(recv_header_t) - sizeof(command_t *) + 1));
#if 0
    LOG_ERROR("n:%d\n",n);
    print_buf(recv_buf, n);
    LOG_ERROR("[Handler]: index:%02x slaver_addr:%02x master_addr:%02x function:%02x\n",
          recv_header->index,
          recv_header->slaver_addr,
          recv_header->master_addr,
          recv_header->function);
    LOG_ERROR("command->nums : %d\n",command->nums);
    for(int i = 0; i < command->nums; i++)
    {
        LOG_ERROR("support command: %02x\n",command->command[i]);
    }
#endif
    recv_header->command = command;
    bucket_t * bucket = &so_uart->bucket[recv_header->index];
    struct blist * found = NULL;
    uint8_t buffer[] = "LIST NOT FOUND";
    pthread_mutex_lock(&bucket->lock);
    {
        struct list_head * p, * list;
        struct blist * node;
        struct list_head * next;
        list = &bucket->list;
        /* 在某个列表里查找，找不到就返回一个找不到的值 */
        list_for_each_safe(p, next, list)
        {   
            node = list_entry(p,struct blist,listhead); 
            if(compare_blist(node,recv_header) == 0)
            {
                list_del(&node->listhead);
                --bucket->count;
                found = node;
                break;
            }
        }  
    }
    pthread_mutex_unlock(&bucket->lock);
    if(found != NULL)
    {
        message_t * message = (message_t *)found->data;
        uint16_t length = message->length;
        uint8_t * data     = message->data;
        if(write(fd, data, length) != length)
        {
            LOG_ERROR("found and write back error\n");
        }
        free(message->data);
        free(message);
        free(found);
    }
    else
    {
        /* 没找到 */
        if(write(fd, buffer, sizeof(buffer)) != sizeof(buffer))
        {
            LOG_ERROR("not found and write back error\n");
        }
    }
    free(arg);
    free(command);
    close(fd);
    return NULL;
}
#endif
void * socket_uart_recv_manager(void * arg)
{
    int listen_fd = open_server_socket(UNIX_SOCKET_UART_RECV);
    pthread_t real_recv;
    int result = pthread_create(&real_recv,
                                NULL,
                                UartRecvWorker,
                                NULL);
    assert(result == 0);
    int len = -1;
    struct sockaddr_un clt_addr;  
    int com_fd = -1;
    message_t * message ;
    struct timeval now;
    uint8_t buffer[MAX_BUFSIZ];
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
        char recv_buf[MAX_BUFSIZ];   
        if( (n = read(com_fd,recv_buf,sizeof(recv_buf))) < 0)
        {
            LOG_ERROR("[%s] Read Error\n",__func__);
        }
        if(isInvalidMessage(recv_buf, n))
        {
            ProcessMessageInvalidMessage(com_fd);
            close(com_fd);
        }
    }
    return NULL;
}
int socket_uart_init(config_t * config)
{
    VERIFY(context,"socket context is NULL");
    VERIFY(config,"socket config is NULL");
    VERIFY(config->devs,"config devs is NULL");

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
        context->devs[i].fd = init_uart_device(&context->devs[i]);
        if(context->devs[i].fd < 0)
        {
            LOG_ERROR("%s open failed",context->devs[i].name);
        }
    }
    context->configed = 1;
}
