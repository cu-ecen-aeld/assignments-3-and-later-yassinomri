CC = gcc
CFLAGS = -Wall -Werror
TARGET = writer
CROSS_COMPILE ?=
SRC = finder-app/writer.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

