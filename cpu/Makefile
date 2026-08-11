IDIR=./src
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wextra -g
BUILD_DIR=build

SRC=$(wildcard src/*.c) main.c
OBJ=$(SRC:%.c=$(BUILD_DIR)/%.o)
TARGET=./bin/6502

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) -o $@ $^
	$(CC) -shared -o $(dir $@)/lib6502.so $^

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf $(BUILD_DIR) $(dir $(TARGET))

rebuild: clean all

.PHONY: clean rebuild