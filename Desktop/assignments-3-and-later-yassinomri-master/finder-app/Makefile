CC = gcc
CFLAGS = -Wall

all: writer

# Check if CROSS_COMPILE is specified
ifeq ($(CROSS_COMPILE),)
# Native compilation
TARGET = writer
else
# Cross-compilation
CC := $(CROSS_COMPILE)$(CC)
TARGET = writer_cross
endif

writer: writer.c
	$(CC) $(CFLAGS) -o $(TARGET) $^

clean :
	rm -f writer writer_cross writer.o writer_cross.o
