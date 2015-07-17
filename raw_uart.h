#ifndef PSOC_TEMP_H
#define PSOC_TEMP_H
void get_time(char *str_t);
int raw_uart_send(int fd, message_t * message);
int init_uart_device(const char * device);
#endif
