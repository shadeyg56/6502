IDIR=./cpu
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wextra
BUILD_DIR=build

SRC=$(wildcard cpu/*.c) main.c
OBJ=$(SRC:%.c=$(BUILD_DIR)/%.o)
TARGET=./bin/6502

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf $(BUILD_DIR) $(dir $(TARGET))

.PHONY: clean