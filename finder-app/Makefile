CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc
CFLAGS = -std=c99
TARGET = writer
OBJS = writer.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): writer.c
	$(CC) $(CFLAGS) -c writer.c

clean:
	rm -f $(TARGET) $(OBJS)
