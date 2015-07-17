#include "multiuart_common.h"
#define DATA_LEN            16
#define TEMP_CMD            0x0A
#define FORWARD_CMD         0x0C
#define STATUS_IDLE         0
#define STATUS_START        1
#define STATUS_MASTER_ADDR  2
#define STATUS_FUNC         3
#define STATUS_CHECKSUM     4
#define STATUS_SLAVE_ADDR   5
#define STATUS_RQSEQ        6
#define STATUS_COMMAND      7
#define STATUS_COMPLETE     8
#define STATUS_DATA         9
#define STATUS_STOP         10
#define UART_START          0xA0
#define UART_STOP           0xA5
#define UART_MASTER_ADDR    0x51
#define UART_SLAVE_ADDR     0x93
#define UART_COMPLETE       0x00
#define BLOCK_SIZE          3
void get_time(char *str_t)
{
    time_t now;
    char str[30];
    memset(str,0,sizeof(str));
    time(&now);
    strftime(str,30,"%Y-%m-%d %H:%M:%S",localtime(&now));
    int len = strlen(str);
    str[len] = '\0';
    strcpy(str_t,str);
}
int raw_uart_send(int fd, message_t * message)
{
    int result = write(fd,message->data,message->length);
    usleep(1000 * 500);
    return result;
}
int init_uart_device(const char * device)
{
	int fd = -1;
    int result = -1;
	struct termios ts;
	char time_str[30];
	int rbaud = 115200;
	/** Open the device. */
	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		get_time(time_str);
		LOG_ERROR("[Console-host-%s]: Open the device %s failed: %s\n",
				time_str, device, strerror(errno));
		return -1;
	}
	/** Set the speed to 115200. */
	result = tcgetattr(fd, &ts);
	if (result)
	{
		get_time(time_str);
		LOG_ERROR("[Console-host-%s]: Get the attribute of device %s failed: %s\n",
				time_str, device, strerror(errno));
		return -1;		
	}
	tcflush(fd, TCIOFLUSH);
	result = cfsetispeed(&ts, B115200); 
	if (result)
	{
		get_time(time_str);
		LOG_ERROR("[Console-host-%s]: Set the input Baul rate %d failed: %s\n",
				time_str, rbaud, strerror(errno));
		return -1;
	}
	result = cfsetospeed(&ts, B115200); 
	if (result) 
	{
		get_time(time_str);
		LOG_ERROR("[Console-host-%s]: Set the output Baul rate %d failed: %s\n",
				time_str, rbaud, strerror(errno));
		return -1;
	}
	tcsetattr(fd, TCSANOW, &ts);
	tcflush(fd,TCIOFLUSH);
	/** Set parity of the uart. */
	result = tcgetattr(fd, &ts);
	assert(!result);
	ts.c_cflag &= ~(CSIZE | PARENB | CRTSCTS | CSTOP);
    ts.c_iflag &= ~(IXON | IXOFF | IXANY | INLCR | ICRNL | INPCK);
    ts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | NOFLSH);
    ts.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    ts.c_cflag |= (CREAD | CS8);
    tcflush(fd,TCIFLUSH);
    ts.c_cc[VTIME] = 0; 
    ts.c_cc[VMIN] = BLOCK_SIZE;
	tcsetattr(fd, TCSANOW, &ts);	
	get_time(time_str);
	LOG_NOTICE("[Console-host-%s]: Host is ready to test the rbaud: %d\n",
			time_str, rbaud);
    return fd;
}  
