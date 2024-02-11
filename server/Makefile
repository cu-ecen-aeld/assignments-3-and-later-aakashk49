CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc
CFLAGS = -std=c99 -Wall
TARGET = aesdsocket
OBJS = aesdsocket.o

all: $(TARGET)
default: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lnsl -lc

$(OBJS): aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c

clean:
	rm -f $(TARGET) $(OBJS)
