CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g

all: writer

writer: writer.o
	$(CC) $(CFLAGS) writer.c -o writer


executible = writer

clean: 
	rm -f $(executible) *.o *.elf *.map
