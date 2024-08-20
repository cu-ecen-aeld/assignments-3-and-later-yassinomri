# Default to native compiler if CROSS_COMPILE is not set
CROSS_COMPILE ?= 
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra
TARGET = aesdsocket
SRC = aesdsocket.c

# Compile the target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Define the 'all' and 'default' targets to build the program
all: $(TARGET)

default: $(TARGET)

# Clean up the build artifacts
.PHONY: clean
clean:
	rm -f $(TARGET)

