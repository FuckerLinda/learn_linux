CC = gcc
AR = ar
CFLAGS = -c

all: libfunc.a

libfunc.a: func.o
	$(AR) rcs $@ $<

func.o: func.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.a
