#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <math.h>
#include <string.h>
#include "sandpile/include/out.h"

int** initialize_grid(int rows, int cols, int center_value, int default_value) {
    // Add padding of 1 cell on each side
    int padded_rows = rows + 2;
    int padded_cols = cols + 2;
    
    // Allocate contiguous memory for better cache performance
    int* data = (int*)calloc(padded_rows * padded_cols, sizeof(int));
    if (data == NULL) {
        printf("Memory allocation failed for grid data\n");
        return NULL;
    }
    
    int** grid = (int**)malloc(padded_rows * sizeof(int*));
    if (grid == NULL) {
        printf("Memory allocation failed for grid rows\n");
        free(data);
        return NULL;
    }
    
    // Set up row pointers for contiguous memory access
    for (int i = 0; i < padded_rows; i++) {
        grid[i] = data + i * padded_cols;
    }
    
    // Initialize only the inner grid (padding remains 0)
    for (int i = 1; i <= rows; i++) {
        for (int j = 1; j <= cols; j++) {
            grid[i][j] = default_value;
        }
    }
    
    // Set center element to center_value (accounting for padding)
    int center_row = rows / 2 + 1;  // +1 for padding offset
    int center_col = cols / 2 + 1;
    grid[center_row][center_col] = center_value;
    
    return grid;
}

void free_grid(int** grid, int padded_rows) {
    if (grid) {
        free(grid[0]); // Free the contiguous data block
        free(grid);    // Free the row pointers
    }
}

void print_grid(int** grid, int rows, int cols) {
    // Print only the inner grid (skip padding)
    for (int i = 1; i <= rows; i++) {
        for (int j = 1; j <= cols; j++) {
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
}

// Optimized tile processing with reduced atomic operations
int process_tile(int** grid, int tile_row, int tile_col, 
                  int tile_size, int rows, int cols) {
    int start_row = tile_row * tile_size + 1;  // +1 for padding offset
    int end_row = start_row + tile_size;
    if (end_row > rows + 1) end_row = rows + 1;  // Don't exceed inner grid
    
    int start_col = tile_col * tile_size + 1;  // +1 for padding offset
    int end_col = start_col + tile_size;
    if (end_col > cols + 1) end_col = cols + 1;  // Don't exceed inner grid
    
    int changed = 1;
    int any_toppled = 0;
    
    while (changed) {
        changed = 0;
        
        // Process cells in cache-friendly order
        for (int i = start_row; i < end_row; i++) {
            for (int j = start_col; j < end_col; j++) {
                if (grid[i][j] >= 4) {
                    any_toppled = 1;
                    
                    int dist = grid[i][j] >> 2;  //division by 4
                    grid[i][j] &= 3;  // modulo 4
                    
                    // Use atomic operations for boundary updates
                    #pragma omp atomic
                    grid[i-1][j] += dist;  // Up
                    
                    #pragma omp atomic  
                    grid[i+1][j] += dist;  // Down
                    
                    #pragma omp atomic
                    grid[i][j-1] += dist;  // Left
                    
                    #pragma omp atomic
                    grid[i][j+1] += dist;  // Right
                    
                    changed = 1;
                }
            }
        }
    }
    return any_toppled;
}

void parallel_sandpile(int** grid, int rows, int cols) {
    int num_threads = omp_get_max_threads();
    
    // Optimize tile size for better cache locality and load balancing
    int target_tiles = 4 * num_threads;  // More tiles for better load balancing
    int tile_size = (int)sqrt((double)(rows * cols) / target_tiles);
    

    if (tile_size < 8) tile_size = 8; // set minimum tile size to prevent excessive switiching (less cache misses)
    if (tile_size > 32) tile_size = 32;  // set maximum tile size to ensure full tile can fit in cache
    
    int tiles_rows = (rows + tile_size - 1) / tile_size;
    int tiles_cols = (cols + tile_size - 1) / tile_size;
    
    printf("Using %d threads\n", num_threads);
    
    int global_changed = 1;
    int iteration = 0;
    
    // Pre-allocate arrays for tile indices to reduce overhead
    int total_tiles = tiles_rows * tiles_cols;
    int* red_tiles = (int*)malloc(total_tiles * sizeof(int));
    int* black_tiles = (int*)malloc(total_tiles * sizeof(int));
    int red_count = 0, black_count = 0;
    
    // Pre-compute red and black tile indices
    for (int tile_idx = 0; tile_idx < total_tiles; tile_idx++) {
        int tile_row = tile_idx / tiles_cols;
        int tile_col = tile_idx % tiles_cols;
        
        if ((tile_row + tile_col) % 2 == 0) {
            red_tiles[red_count++] = tile_idx;
        } else {
            black_tiles[black_count++] = tile_idx;
        }
    }
    
    while (global_changed) {
        global_changed = 0;
        iteration++;
        
        // Process RED tiles
        #pragma omp parallel reduction(||:global_changed)
        {
            int local_changed = 0;
            
            #pragma omp for schedule(dynamic) 
            for (int i = 0; i < red_count; i++) {
                int tile_idx = red_tiles[i];
                int tile_row = tile_idx / tiles_cols;
                int tile_col = tile_idx % tiles_cols;
                
                if (process_tile(grid, tile_row, tile_col, tile_size, rows, cols)) {
                    local_changed = 1;
                }
            }
            
            if (local_changed) global_changed = 1;
        }
        
        // Process BLACK tiles
        #pragma omp parallel reduction(||:global_changed)
        {
            int local_changed = 0;
            
            #pragma omp for schedule(static) nowait
            for (int i = 0; i < black_count; i++) {
                int tile_idx = black_tiles[i];
                int tile_row = tile_idx / tiles_cols;
                int tile_col = tile_idx % tiles_cols;
                
                if (process_tile(grid, tile_row, tile_col, tile_size, rows, cols)) {
                    local_changed = 1;
                }
            }
            
            if (local_changed) global_changed = 1;
        }
        
    }
    
    printf("Sandpile stabilized after %d iterations\n", iteration - 1);
    
    free(red_tiles);
    free(black_tiles);
}

int main(int argc, char* argv[]) {
    int rows = 60;
    int cols = 30;
    int center_value = 12121;
    int default_value = 624;
        
    if (argc == 5){
        rows = atoi(argv[1]);
        cols = atoi(argv[2]);
        center_value = atoi(argv[3]);
        default_value = atoi(argv[4]);
    }
    
    if (rows <= 0 || cols <= 0) {
        printf("Error: rows and columns must be positive integers\n");
        return 1;
    }
    
    printf("Initializing %dx%d grid with %d threads...\n", rows, cols, omp_get_max_threads());
    
    int** grid = initialize_grid(rows, cols, center_value, default_value);
    if (grid == NULL) {
        return 1;
    }
    
    printf("Running parallel sandpile simulation...\n");
    double start_time = omp_get_wtime();
    
    parallel_sandpile(grid, rows, cols);
    
    double end_time = omp_get_wtime();

    double time = end_time - start_time;
    printf("Simulation completed in %.4f seconds\n", time);
    
    // printf("\nFinal stable configuration:\n");
    // print_grid(grid, rows, cols);
    bool mpi = false;
    vis_grid(grid, "output_openmp.ppm", rows, cols, mpi);
    write_results("output.txt", "OpemMP", 4, rows, cols, center_value, default_value, time);
    
    free_grid(grid, rows + 2);  // Free padded grid
    return 0;
}