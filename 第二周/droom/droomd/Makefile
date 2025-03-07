# droomd/Makefile
CC = gcc
CFLAGS = -I$(TOPSRCDIR)/include
SRC = src/daemon.c
TARGET = droomd

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

install:
	install -m 755 $(TARGET) $(DESTDIR)

clean:
	rm -f $(TARGET)
