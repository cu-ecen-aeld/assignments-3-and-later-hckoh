CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Werror -g

all: aesdsocket.c
	$(CC) $(CFLAGS) aesdsocket.c -o aesdsocket

executible = aesdsocket

clean: 
	rm -f $(executible) *.o *.elf *.map
