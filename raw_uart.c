#include "multiuart_common.h"
int raw_uart_send(int fd, message_t * message)
{
    int result = write(fd,message->data,message->length);
    usleep(1000 * 500);
    return result;
}
int init_uart_device(uart_dev_t * dev)
{
	int fd = -1;
    int result = -1;
	struct termios ts;
	char time_str[30];
	int rbaud = dev->rbaud;
	/** Open the device. */
	fd = open(dev->name, O_RDWR);
	if (fd < 0)
	{
		LOG_ERROR("[MULTIUART]: Open the device %s failed: %s\n",
				 dev->name, strerror(errno));
		return -1;
	}
	/** Set the speed to 115200. */
	result = tcgetattr(fd, &ts);
	if (result)
	{
		LOG_ERROR("[MULTIUART]: Get the attribute of device %s failed: %s\n",
				 dev->name, strerror(errno));
		return -1;		
	}
    memset(&ts, 0, sizeof(struct termios));
    ts.c_cflag = CREAD | HUPCL | CLOCAL;
	tcflush(fd, TCIOFLUSH);
    if(rbaud == 115200)
    {
	    result = cfsetispeed(&ts, B115200); 
	    if (result)
    	{
    		printf("[MULTIUART]: Set the input Baul rate %d failed: %s\n",
    				 rbaud, strerror(errno));
    		return -1;
    	}
    	result = cfsetospeed(&ts, B115200); 
    	if (result) 
    	{
    		printf("[MULTIUART]: Set the output Baul rate %d failed: %s\n",
    				 rbaud, strerror(errno));
    		return -1;
    	}
    }
    else if(rbaud == 9600)
    {
	    result = cfsetispeed(&ts, B9600); 
	    if (result)
    	{
    		printf("[MULTIUART]: Set the input Baul rate %d failed: %s\n",
    				 rbaud, strerror(errno));
    		return -1;
    	}
    	result = cfsetospeed(&ts, B9600); 
    	if (result) 
    	{
    		printf("[MULTIUART]: Set the output Baul rate %d failed: %s\n",
    				 rbaud, strerror(errno));
    		return -1;
    	}
    }
    // 8N1
    ts.c_cflag &= ~PARENB;
    ts.c_cflag &= ~CSTOPB;
    ts.c_cflag &= ~CSIZE;
    ts.c_cflag |= CS8;
    // hw flow control off
    ts.c_cflag &= ~CRTSCTS;
    // sw flow control off
    ts.c_iflag &= ~(IXON | IXOFF | IXANY);
    ts.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &ts);
	printf("[MULTIUART]: Host is ready to test the rbaud: %d\n",
    			 rbaud);
    return fd;
}  
