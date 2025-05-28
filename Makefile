# Compiler and flags
CC = gcc
# -Wall to enable common warning and -g tells compiler to include
# debugging info in compiled lib
CFLAGS = -Iinclude -Wall -g

# Directories
SRC_DIR = serial/src
BIN_DIR = serial/bin

# Source files
SRCS = $(SRC_DIR)/grid.c $(SRC_DIR)/sandpile.c main.c

# Output executable
TARGET = $(BIN_DIR)/main

# Default target
all: $(TARGET)

# Create executable
$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Run main with ARGS="rows cols centre allValues
run: all
	./$(TARGET) $(ARGS)

# Clean up build files
clean:
	rm -rf $(BIN_DIR)