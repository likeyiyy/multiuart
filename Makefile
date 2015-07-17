# Default target.
all:

ifdef TILERA_ROOT

CC = $(TILERA_ROOT)/bin/tile-cc
TILE_MONITOR = $(TILERA_ROOT)/bin/tile-monitor
PARALLELIZE = $(TILERA_ROOT)/bin/parallelize
else

CC = gcc

endif

CFLAGS = -std=gnu99 -Wall -g $(OPT)
LIBS = -lgxio -lpthread

SRCS = uart_daemon.c socket_uart.c raw_uart.c llog.c

OBJS = $(SRCS:%.c=%.o) 

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) $(XFLAGS) -c -o $@ $<    


uart_daemon: uart_daemon.o socket_uart.o raw_uart.o llog.o
	$(CC) $(LDFLAGS) $(XFLAGS) -o $@ $^ $(LIBS)

all: $(OBJS) uart_daemon

clean:
	rm -f *.o $(OBJS) example uart_daemon

.PHONY: all clean

