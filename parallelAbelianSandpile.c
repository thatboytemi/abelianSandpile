#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define THRESHOLD 4  // Sandpile threshold for toppling

int** initialize_grid(int rows, int cols, int center_value, int default_value) {
    // Add padding of 1 cell on each side
    int padded_rows = rows + 2;
    int padded_cols = cols + 2;
    
    int** grid = (int**)malloc(padded_rows * sizeof(int*));
    if (grid == NULL) {
        printf("Memory allocation failed for grid rows\n");
        return NULL;
    }
    
    for (int i = 0; i < padded_rows; i++) {
        grid[i] = (int*)calloc(padded_cols, sizeof(int)); // calloc initializes to 0
        if (grid[i] == NULL) {
            printf("Memory allocation failed for grid row %d\n", i);
            for (int j = 0; j < i; j++) {
                free(grid[j]);
            }
            free(grid);
            return NULL;
        }
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
    for (int i = 0; i < padded_rows; i++) {
        free(grid[i]);
    }
    free(grid);
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

// Check if a tile needs processing (has any cell >= threshold)
int tile_needs_processing(int** grid, int tile_row, int tile_col, 
                         int tile_size, int rows, int cols) {
    int start_row = tile_row * tile_size + 1;  // +1 for padding offset
    int end_row = start_row + tile_size;
    if (end_row > rows + 1) end_row = rows + 1;  // Don't exceed inner grid
    
    int start_col = tile_col * tile_size + 1;  // +1 for padding offset
    int end_col = start_col + tile_size;
    if (end_col > cols + 1) end_col = cols + 1;  // Don't exceed inner grid
    
    for (int i = start_row; i < end_row; i++) {
        for (int j = start_col; j < end_col; j++) {
            if (grid[i][j] >= THRESHOLD) {
                return 1;
            }
        }
    }
    return 0;
}

// Process a single tile until it's stable
void process_tile(int** grid, int tile_row, int tile_col, 
                  int tile_size, int rows, int cols) {
    int start_row = tile_row * tile_size + 1;  // +1 for padding offset
    int end_row = start_row + tile_size;
    if (end_row > rows + 1) end_row = rows + 1;  // Don't exceed inner grid
    
    int start_col = tile_col * tile_size + 1;  // +1 for padding offset
    int end_col = start_col + tile_size;
    if (end_col > cols + 1) end_col = cols + 1;  // Don't exceed inner grid
    
    int changed = 1;
    int dist = 0;
    while (changed) {
        changed = 0;
        
        for (int i = start_row; i < end_row; i++) {
            for (int j = start_col; j < end_col; j++) {
                if (grid[i][j] >= THRESHOLD) {
                    // Topple this cell with atomic operations
                    dist = grid[i][j] / 4;
                    grid[i][j] %= 4;
                    
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
}

void parallel_sandpile(int** grid, int rows, int cols) {
    int num_threads = omp_get_max_threads();
    
    // Calculate optimal tile size based on number of threads
    // We want approximately 2 * num_threads tiles (since we alternate red/black)
    int total_tiles = 2 * num_threads;
    int tile_size = (int)sqrt((double)(rows * cols) / total_tiles);
    if (tile_size < 1) tile_size = 1;
    
    int tiles_rows = (rows + tile_size - 1) / tile_size;
    int tiles_cols = (cols + tile_size - 1) / tile_size;
    
    // printf("Using %d threads with tile size %dx%d (%dx%d tiles)\n", 
    //        num_threads, tile_size, tile_size, tiles_rows, tiles_cols);
    
    int global_changed = 1;
    int iteration = 0;
    
    while (global_changed) {
        global_changed = 0;
        iteration++;
        
        // Process RED tiles (where tile_row + tile_col is even)
        #pragma omp parallel
        {
            int local_changed = 0;
            
            #pragma omp for schedule(dynamic)
            for (int tile_idx = 0; tile_idx < tiles_rows * tiles_cols; tile_idx++) {
                int tile_row = tile_idx / tiles_cols;
                int tile_col = tile_idx % tiles_cols;
                
                
                if ((tile_row + tile_col) % 2 == 0) {
                    if (tile_needs_processing(grid, tile_row, tile_col, tile_size, rows, cols)) {
                        process_tile(grid, tile_row, tile_col, tile_size, rows, cols);
                        local_changed = 1;
                    }
                }
            }
            
            #pragma omp critical
            {
                if (local_changed) global_changed = 1;
            }
        }
        
        
        // Process BLACK tiles (where tile_row + tile_col is odd)
        #pragma omp parallel
        {
            int local_changed = 0;
            
            #pragma omp for schedule(dynamic)
            for (int tile_idx = 0; tile_idx < tiles_rows * tiles_cols; tile_idx++) {
                int tile_row = tile_idx / tiles_cols;
                int tile_col = tile_idx % tiles_cols;
                
                if ((tile_row + tile_col) % 2 == 1) {
                    if (tile_needs_processing(grid, tile_row, tile_col, tile_size, rows, cols)) {
                        process_tile(grid, tile_row, tile_col, tile_size, rows, cols);
                        local_changed = 1;
                    }
                }
            }
            
            #pragma omp critical
            {
                if (local_changed) global_changed = 1;
            }
        }
    }
    
    printf("Sandpile stabilized after %d iterations\n", iteration - 1);
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
    
    int** grid = initialize_grid(rows, cols, center_value, default_value);
    if (grid == NULL) {
        return 1;
    }
    
    // printf("Initial %dx%d grid:\n", rows, cols);
    // print_grid(grid, rows, cols);
    // printf("\n");
    
    printf("Running parallel sandpile simulation...\n");
    double start_time = omp_get_wtime();
    
    parallel_sandpile(grid, rows, cols);
    
    double end_time = omp_get_wtime();
    printf("Simulation completed in %.4f seconds\n", end_time - start_time);
    
    printf("\nFinal stable configuration:\n");
    print_grid(grid, rows, cols);
    
    free_grid(grid, rows + 2);  // Free padded grid
    return 0;
}