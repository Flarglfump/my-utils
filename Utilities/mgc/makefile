# Build mgc into ../bin. Works from any directory: make -C Utilities/mgc
SRC_DIR  := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BIN_DIR  := $(SRC_DIR)../bin
TARGET   := $(BIN_DIR)/mgc

CXX      ?= c++
CXXFLAGS ?= -O2
CXXFLAGS += -Wall -Wextra -std=c++17

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC_DIR)mgc.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) "$<" -o "$@"

$(BIN_DIR):
	mkdir -p "$@"

clean:
	rm -f "$(TARGET)"
