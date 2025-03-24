CC = gcc
CFLAGS = -Wall -Wextra -g 

all: csvreader

csvreader: csvreader.c io.c 
	$(CC) $(CFLAGS) csvreader.c io.c -o csvreader -lm

clean:
	-rm -f *.o 

purge: clean
	-rm -f csvreader
	-rm -f *.swp
