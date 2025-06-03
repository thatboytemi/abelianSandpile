#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <math.h>
#include <string.h>
#include "sandpile/include/out.h"

int** initialize_grid(int rows, int cols, int center_value, int default_value) {
    int padded_rows = rows + 2;
    int padded_cols = cols + 2;
    
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
    
    for (int i = 0; i < padded_rows; i++) {
        grid[i] = data + i * padded_cols;
    }
    
    for (int i = 1; i <= rows; i++) {
        for (int j = 1; j <= cols; j++) {
            grid[i][j] = default_value;
        }
    }
    
    int center_row = rows / 2 + 1;
    int center_col = cols / 2 + 1;
    grid[center_row][center_col] = center_value;
    
    return grid;
}

void free_grid(int** grid, int padded_rows) {
    if (grid) {
        free(grid[0]);
        free(grid);
    }
}

void print_grid(int** grid, int rows, int cols) {
    for (int i = 1; i <= rows; i++) {
        for (int j = 1; j <= cols; j++) {
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
}

int process_tile(int** grid, int tile_row, int tile_col, int tile_size, int rows, int cols) {
    int start_row = tile_row * tile_size + 1;
    int end_row = start_row + tile_size;
    if (end_row > rows + 1) end_row = rows + 1;
    
    int start_col = tile_col * tile_size + 1;
    int end_col = start_col + tile_size;
    if (end_col > cols + 1) end_col = cols + 1;
    
    int changed = 1;
    int any_toppled = 0;
    
    // Keep processing until no more changes in this tile
    while (changed) {
        changed = 0;
        
        // Process cells in cache-friendly order
        for (int i = start_row; i < end_row; i++) {
            for (int j = start_col; j < end_col; j++) {
                if (grid[i][j] >= 4) {
                    any_toppled = 1;
                    
                    int dist = grid[i][j] >> 2;  // division by 4
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



// Optimized red-black tiling with better scheduling
void parallel_sandpile(int** grid, int rows, int cols) {
    int num_threads = omp_get_max_threads() - 6;
    
    // Smaller tiles for better load balancing
    int tile_size = 16; // Fixed optimal size for most cases
    int tiles_rows = (rows + tile_size - 1) / tile_size;
    int tiles_cols = (cols + tile_size - 1) / tile_size;
    
    printf("Using %d threads \n", num_threads);
    
    // We don't actually need temp grids for the corrected version
    // Keeping the parameter for consistency but not using it
    
    int global_changed = 1;
    int iteration = 0;
    
    // Pre-compute tile indices
    int total_tiles = tiles_rows * tiles_cols;
    int* red_tiles = (int*)malloc(total_tiles * sizeof(int));
    int* black_tiles = (int*)malloc(total_tiles * sizeof(int));
    int red_count = 0, black_count = 0;
    
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
            
            #pragma omp for schedule(guided, 2) nowait
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
        
        // Implicit barrier here
        
        // Process BLACK tiles
        #pragma omp parallel reduction(||:global_changed)
        {

            int local_changed = 0;
            
            #pragma omp for schedule(guided, 2) nowait
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
    
    // No cleanup needed since we're not using temp grids
    
    free(red_tiles);
    free(black_tiles);
}

int main(int argc, char* argv[]) {
    int rows = 60;
    int cols = 30;
    int center_value = 12121;
    int default_value = 624;
    
    if (argc >= 5){
        rows = atoi(argv[1]);
        cols = atoi(argv[2]);
        center_value = atoi(argv[3]);
        default_value = atoi(argv[4]);
    }
    
    if (rows <= 0 || cols <= 0) {
        printf("Error: rows and columns must be positive integers\n");
        return 1;
    }
    
    printf("Initializing %dx%d grid...\n", rows, cols);
    
    int** grid = initialize_grid(rows, cols, center_value, default_value);
    if (grid == NULL) {
        return 1;
    }
    
    printf("Running sandpile simulation ...\n");
    double start_time = omp_get_wtime();
    
    parallel_sandpile(grid, rows, cols);
    
    
    double end_time = omp_get_wtime();
    printf("Simulation completed in %.4f seconds\n", end_time - start_time);
    double time = end_time - start_time;
    bool mpi = false;
    vis_grid(grid, "output_openmp.ppm", rows, cols, mpi);
    const char *filepath = "/mnt/lustre/users/student42/HPC_A1/results.csv";
    write_results(filepath, "OpemMP", omp_get_max_threads(), rows, cols, center_value, default_value, time);

}