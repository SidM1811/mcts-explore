#!/bin/bash

# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -g -std=c++17 -pthread

# Source files
SRC_DIR = ../src
SOURCE_FILES = mcts.cpp

SRCS = $(SOURCE_FILES:%=$(SRC_DIR)/%)

# Build directory
BUILD_DIR = obj
EXEC_DIR = bin

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executable
EXEC = $(EXEC_DIR)/chessbot

# Default target
all: $(EXEC)

# Link object files to create executable
$(EXEC): $(OBJS) | $(EXEC_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	chmod +x $@

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Create build and executable directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXEC_DIR):
	mkdir -p $(EXEC_DIR)

# Include dependencies
-include $(OBJS:.o=.d)

# Clean up build files
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(EXEC)

.PHONY: all clean
