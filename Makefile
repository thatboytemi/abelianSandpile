# Compiler and flags
CC = gcc
MPICC = mpicc
# -Wall to enable common warning and -g tells compiler to include
# debugging info in compiled lib
CFLAGS = -Iinclude -Wall -g
# Add OpenMP flags for parallel version
PARALLEL_CFLAGS = $(CFLAGS) -Xpreprocessor -fopenmp -L/opt/homebrew/opt/libomp/lib -I/opt/homebrew/opt/libomp/include -lomp
# MPI flags
MPI_CFLAGS = $(CFLAGS)

# Directories
SRC_DIR = serial/src
BIN_DIR = serial/bin
PARALLEL_BIN_DIR = parallel/bin
MPI_BIN_DIR = mpi/bin

# Source files
SRCS = $(SRC_DIR)/grid.c $(SRC_DIR)/sandpile.c main.c
PARALLEL_SRCS = $(SRC_DIR)/grid.c $(SRC_DIR)/sandpile.c parallelAbelianSandpile.c
MPI_SRCS = $(SRC_DIR)/grid.c $(SRC_DIR)/sandpile.c mpiSandpile.c

# Output executables
TARGET = $(BIN_DIR)/main
PARALLEL_TARGET = $(PARALLEL_BIN_DIR)/parallelAbelianSandpile
THREAD_INFO_TARGET = $(BIN_DIR)/thread_info
MPI_TARGET = $(MPI_BIN_DIR)/mpiSandpile

# Default target
all: $(TARGET) $(PARALLEL_TARGET) $(THREAD_INFO_TARGET) $(MPI_TARGET)

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

# Create MPI executable
$(MPI_TARGET): $(MPI_SRCS)
	@mkdir -p $(MPI_BIN_DIR)
	$(MPICC) $(MPI_CFLAGS) $^ -o $@

# Run serial version with ARGS="rows cols centre allValues"
run: all
	./$(TARGET) $(ARGS)

# Run parallel version with ARGS="rows cols centre allValues"
run_parallel: all
	./$(PARALLEL_TARGET) $(ARGS)

# Run thread info program
run_thread_info: $(THREAD_INFO_TARGET)
	./$(THREAD_INFO_TARGET)

# Run MPI version with ARGS="rows cols centre allValues"
run_mpi: $(MPI_TARGET)
	mpirun -np 4 ./$(MPI_TARGET) $(ARGS)

# Clean up build files
clean:
	rm -rf $(BIN_DIR) $(PARALLEL_BIN_DIR) $(MPI_BIN_DIR)
