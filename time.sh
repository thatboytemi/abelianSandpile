#!/bin/bash

# Script to run sandpile experiments with different parameters
# Make bash script executable
# chmod +x time.sh
# Usage: ./time.sh

# Loop through different grid sizes
for i in $(seq 513 913); do
    make run ARGS="$i $i 4 4"
    make run_parallel ARGS="$i $i 4 4"
    make run_mpi ARGS="$i $i 4 4"
done