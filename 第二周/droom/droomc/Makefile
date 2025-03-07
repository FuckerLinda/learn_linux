# droomc/Makefile
CC = gcc
CFLAGS = -I$(TOPSRCDIR)/include
SRC = src/client.c
TARGET = droomc

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

install:
	install -m 755 $(TARGET) $(DESTDIR)

clean:
	rm -f $(TARGET)
