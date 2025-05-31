#!/bin/bash

# Script to run sandpile experiments with different parameters
# Make bash script executable
# chmod +x time.sh
# Usage: ./time.sh

echo "Starting sandpile experiments..."
echo "================================"

# Loop through different grid sizes
# for i in $(seq 413 513); do
i=513
    echo ""
    echo "Running experiments with grid size: ${i}x${i}"
    echo "----------------------------------------"
    
    echo "1. Running serial version..."
    make run ARGS="$i $i 4 4"
    
    echo "2. Running parallel version..."
    make run_parallel ARGS="$i $i 4 4"
    
    echo "3. Running MPI version..."
    make run_mpi ARGS="$i $i 4 4"
    
    echo "Completed grid size ${i}x${i}"
# done

echo ""
echo "All experiments completed!"
