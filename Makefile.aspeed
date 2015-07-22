# Default target.
all:

CC = arm-aspeed-linux-gnu-gcc
LIBS = -lpthread

CFLAGS = -std=gnu99 -Wall -g $(OPT) -I./ -I./protocol

SRCS = uart_daemon.c socket_uart.c raw_uart.c llog.c \
	   message.c queue.c protocol/ipmi_protocol.c \
	   protocol/raw_protocol.c protocol/proto_manager.c \
	   config.c

OBJS = $(SRCS:%.c=%.o) 

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) $(XFLAGS) -c -o $@ $<    


uart_daemon: $(OBJS)
	$(CC) $(LDFLAGS) $(XFLAGS) -o $@ $^ $(LIBS)

all: $(OBJS) uart_daemon

clean:
	rm -f *.o $(OBJS) example uart_daemon

.PHONY: all clean

