CC = gcc
CFLAGS = -I../lib
LDFLAGS = -L../lib -lfunc

all: main

main: main.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f main main.o
