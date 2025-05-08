# Compiler and flags
CC = gcc
CFLAGS = -I ~/GmSSL-3.1.1-Linux/include
LDFLAGS = -L ~/GmSSL-3.1.1-Linux/lib -lgmssl

# Targets
TARGETS = gr gsm3

# Source files
SRCS = testrand.c testsm3.c

# Object files
OBJS = $(SRCS:.c=.o)

# Make targets
all: $(TARGETS)

gr: testrand.c
	$(CC) $(CFLAGS) testrand.c $(LDFLAGS) -o gr

gsm3: testsm3.c
	$(CC) $(CFLAGS) testsm3.c $(LDFLAGS) -o gsm3

clean:
	rm -f $(TARGETS) $(OBJS)
