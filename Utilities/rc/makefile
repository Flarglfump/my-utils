# Build rc into ../bin. Works from any directory: make -C Utilities/rc
SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BIN_DIR := $(SRC_DIR)../bin
TARGET  := $(BIN_DIR)/rc

CC      ?= cc
CFLAGS  ?= -O2
CFLAGS  += -Wall -Wextra -std=c99

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC_DIR)rc.c | $(BIN_DIR)
	$(CC) $(CFLAGS) "$<" -o "$@"

$(BIN_DIR):
	mkdir -p "$@"

clean:
	rm -f "$(TARGET)"
