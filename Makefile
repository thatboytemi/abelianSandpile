# Compiler and flags
CC = gcc
# -Wall to enable common warning and -g tells compiler to include
# debugging info in compiled lib
CFLAGS = -Iinclude -Wall -g
# Add OpenMP flags for parallel version
PARALLEL_CFLAGS = $(CFLAGS) -Xpreprocessor -fopenmp -L/opt/homebrew/opt/libomp/lib -I/opt/homebrew/opt/libomp/include -lomp

# Directories
SRC_DIR = serial/src
BIN_DIR = serial/bin
PARALLEL_BIN_DIR = parallel/bin

# Source files
SRCS = $(SRC_DIR)/grid.c $(SRC_DIR)/sandpile.c main.c
PARALLEL_SRCS = $(SRC_DIR)/grid.c $(SRC_DIR)/sandpile.c parallelAbelianSandpile.c

# Output executables
TARGET = $(BIN_DIR)/main
PARALLEL_TARGET = $(PARALLEL_BIN_DIR)/parallelAbelianSandpile
THREAD_INFO_TARGET = $(BIN_DIR)/thread_info

# Default target
all: $(TARGET) $(PARALLEL_TARGET) $(THREAD_INFO_TARGET)

# Create serial executable
$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Create parallel executable
$(PARALLEL_TARGET): $(PARALLEL_SRCS)
	@mkdir -p $(PARALLEL_BIN_DIR)
	$(CC) $(PARALLEL_CFLAGS) $^ -o $@

# Create thread info executable
$(THREAD_INFO_TARGET): thread_info.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(PARALLEL_CFLAGS) $^ -o $@

# Run serial version with ARGS="rows cols centre allValues"
run: all
	./$(TARGET) $(ARGS)

# Run parallel version with ARGS="rows cols centre allValues"
run_parallel: all
	./$(PARALLEL_TARGET) $(ARGS)

# Run thread info program
run_thread_info: $(THREAD_INFO_TARGET)
	./$(THREAD_INFO_TARGET)

# Clean up build files
clean:
	rm -rf $(BIN_DIR) $(PARALLEL_BIN_DIR)