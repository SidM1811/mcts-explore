#!/bin/bash

# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -O3 -std=c++17 -pthread -DNDEBUG -march=native -flto

# Debug flags (enables asserts and additional debugging)
DEBUG_CFLAGS = -Wall -g -std=c++17 -pthread -DDEBUG -O0 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined

# Source files
SRC_DIR = ../src
SOURCE_FILES = mcts.cpp

SRCS = $(SOURCE_FILES:%=$(SRC_DIR)/%)

# Build directory
BUILD_DIR = obj
EXEC_DIR = bin

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEBUG_OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%-debug.o)

# Executable
EXEC = $(EXEC_DIR)/chessbot
DEBUG_EXEC = $(EXEC_DIR)/chessbot-debug

# Default target
all: $(EXEC)

# Debug target
debug: $(DEBUG_EXEC)

# Link object files to create debug executable
$(DEBUG_EXEC): $(DEBUG_OBJS) | $(EXEC_DIR)
	$(CC) $(DEBUG_CFLAGS) -o $@ $^
	chmod +x $@

# Link object files to create executable
$(EXEC): $(OBJS) | $(EXEC_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	chmod +x $@

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Compile source files to debug object files
$(BUILD_DIR)/%-debug.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(DEBUG_CFLAGS) -MMD -c $< -o $@

# Create build and executable directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXEC_DIR):
	mkdir -p $(EXEC_DIR)

# Include dependencies
-include $(OBJS:.o=.d)
-include $(DEBUG_OBJS:.o=.d)

# Clean up build files
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(EXEC) $(DEBUG_EXEC)

.PHONY: all debug clean
