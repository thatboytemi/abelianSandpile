#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <string.h>



typedef struct {
    int **grid;
    int local_rows, local_cols;
    int global_rows, global_cols;
    int start_row, start_col;
    int rank, size;
    int north_rank, south_rank, east_rank, west_rank;
    int proc_row, proc_col, proc_rows, proc_cols;
} SandpileData;

// Initialize local grid with ghost cells (padding of 1 on all sides)
int** allocate_grid_with_ghosts(int rows, int cols) {
    int **grid = (int**)malloc((rows + 2) * sizeof(int*));
    for (int i = 0; i < rows + 2; i++) {
        grid[i] = (int*)calloc(cols + 2, sizeof(int));
    }
    return grid;
}

void free_grid(int **grid, int rows) {
    for (int i = 0; i < rows + 2; i++) {
        free(grid[i]);
    }
    free(grid);
}

// Setup 2D Cartesian topology for domain decomposition
void setup_domain_decomposition(SandpileData *data) {
    // Create 2D processor grid
    int dims[2] = {0, 0};
    MPI_Dims_create(data->size, 2, dims);
    data->proc_rows = dims[0];
    data->proc_cols = dims[1];
    
    int periods[2] = {0, 0}; // No periodic boundaries
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart_comm);
    
    int coords[2];
    MPI_Cart_coords(cart_comm, data->rank, 2, coords);
    data->proc_row = coords[0];
    data->proc_col = coords[1];
    
    // Calculate local domain size with proper remainder handling
    int base_rows = data->global_rows / data->proc_rows;
    int base_cols = data->global_cols / data->proc_cols;
    int extra_rows = data->global_rows % data->proc_rows;
    int extra_cols = data->global_cols % data->proc_cols;
    
    // Distribute extra rows to first processes
    if (data->proc_row < extra_rows) {
        data->local_rows = base_rows + 1;
        data->start_row = data->proc_row * (base_rows + 1);
    } else {
        data->local_rows = base_rows;
        data->start_row = extra_rows * (base_rows + 1) + (data->proc_row - extra_rows) * base_rows;
    }
    
    // Distribute extra columns to first processes  
    if (data->proc_col < extra_cols) {
        data->local_cols = base_cols + 1;
        data->start_col = data->proc_col * (base_cols + 1);
    } else {
        data->local_cols = base_cols;
        data->start_col = extra_cols * (base_cols + 1) + (data->proc_col - extra_cols) * base_cols;
    }
    
    // Find neighbor ranks
    MPI_Cart_shift(cart_comm, 0, 1, &data->north_rank, &data->south_rank);
    MPI_Cart_shift(cart_comm, 1, 1, &data->west_rank, &data->east_rank);
    
    MPI_Comm_free(&cart_comm);
}

// Initialize the sandpile with center spike
void initialize_sandpile(SandpileData *data, int center_value, int default_value) {
    data->grid = allocate_grid_with_ghosts(data->local_rows, data->local_cols);
    
    // Initialize interior cells with default value (ghost cells remain 0)
    for (int i = 1; i <= data->local_rows; i++) {
        for (int j = 1; j <= data->local_cols; j++) {
            data->grid[i][j] = default_value;
        }
    }
    
    // Set center cell if it's in this process's domain
    int global_center_row = data->global_rows / 2;
    int global_center_col = data->global_cols / 2;
    
    if (global_center_row >= data->start_row && 
        global_center_row < data->start_row + data->local_rows &&
        global_center_col >= data->start_col && 
        global_center_col < data->start_col + data->local_cols) {
        
        int local_center_row = global_center_row - data->start_row + 1; // +1 for ghost cell offset
        int local_center_col = global_center_col - data->start_col + 1;
        data->grid[local_center_row][local_center_col] = center_value;
    }
}


// Collect boundary contributions that went to ghost cells and send them back to neighbors
void collect_boundary_contributions(SandpileData *data) {
    MPI_Request requests[8];
    MPI_Status statuses[8];
    int req_count = 0;
    
    // Allocate buffers for contributions from ghost cells
    int *north_contrib = (int*)calloc(data->local_cols, sizeof(int));
    int *south_contrib = (int*)calloc(data->local_cols, sizeof(int));
    int *west_contrib = (int*)calloc(data->local_rows, sizeof(int));
    int *east_contrib = (int*)calloc(data->local_rows, sizeof(int));
    
    int *north_recv = (int*)calloc(data->local_cols, sizeof(int));
    int *south_recv = (int*)calloc(data->local_cols, sizeof(int));
    int *west_recv = (int*)calloc(data->local_rows, sizeof(int));
    int *east_recv = (int*)calloc(data->local_rows, sizeof(int));
    
    // Collect contributions from ghost cells
    if (data->north_rank != MPI_PROC_NULL) {
        for (int j = 0; j < data->local_cols; j++) {
            north_contrib[j] = data->grid[0][j + 1];
            data->grid[0][j + 1] = 0; // Clear ghost cell
        }
        MPI_Isend(north_contrib, data->local_cols, MPI_INT, data->north_rank, 4, MPI_COMM_WORLD, &requests[req_count++]);
        MPI_Irecv(north_recv, data->local_cols, MPI_INT, data->north_rank, 5, MPI_COMM_WORLD, &requests[req_count++]);
    }
    
    if (data->south_rank != MPI_PROC_NULL) {
        for (int j = 0; j < data->local_cols; j++) {
            south_contrib[j] = data->grid[data->local_rows + 1][j + 1];
            data->grid[data->local_rows + 1][j + 1] = 0; // Clear ghost cell
        }
        MPI_Isend(south_contrib, data->local_cols, MPI_INT, data->south_rank, 5, MPI_COMM_WORLD, &requests[req_count++]);
        MPI_Irecv(south_recv, data->local_cols, MPI_INT, data->south_rank, 4, MPI_COMM_WORLD, &requests[req_count++]);
    }
    
    if (data->west_rank != MPI_PROC_NULL) {
        for (int i = 0; i < data->local_rows; i++) {
            west_contrib[i] = data->grid[i + 1][0];
            data->grid[i + 1][0] = 0; // Clear ghost cell
        }
        MPI_Isend(west_contrib, data->local_rows, MPI_INT, data->west_rank, 6, MPI_COMM_WORLD, &requests[req_count++]);
        MPI_Irecv(west_recv, data->local_rows, MPI_INT, data->west_rank, 7, MPI_COMM_WORLD, &requests[req_count++]);
    }
    
    if (data->east_rank != MPI_PROC_NULL) {
        for (int i = 0; i < data->local_rows; i++) {
            east_contrib[i] = data->grid[i + 1][data->local_cols + 1];
            data->grid[i + 1][data->local_cols + 1] = 0; // Clear ghost cell
        }
        MPI_Isend(east_contrib, data->local_rows, MPI_INT, data->east_rank, 7, MPI_COMM_WORLD, &requests[req_count++]);
        MPI_Irecv(east_recv, data->local_rows, MPI_INT, data->east_rank, 6, MPI_COMM_WORLD, &requests[req_count++]);
    }
    
    // Wait for all communications to complete
    MPI_Waitall(req_count, requests, statuses);
    
    // Add received contributions to boundary cells
    if (data->north_rank != MPI_PROC_NULL) {
        for (int j = 0; j < data->local_cols; j++) {
            data->grid[1][j + 1] += north_recv[j]; // Add to first internal row
        }
    }
    
    if (data->south_rank != MPI_PROC_NULL) {
        for (int j = 0; j < data->local_cols; j++) {
            data->grid[data->local_rows][j + 1] += south_recv[j]; // Add to last internal row
        }
    }
    
    if (data->west_rank != MPI_PROC_NULL) {
        for (int i = 0; i < data->local_rows; i++) {
            data->grid[i + 1][1] += west_recv[i]; // Add to first internal column
        }
    }
    
    if (data->east_rank != MPI_PROC_NULL) {
        for (int i = 0; i < data->local_rows; i++) {
            data->grid[i + 1][data->local_cols] += east_recv[i]; // Add to last internal column
        }
    }
    
    // Free buffers
    free(north_contrib); free(south_contrib); free(north_recv); free(south_recv);
    free(west_contrib); free(east_contrib); free(west_recv); free(east_recv);
}

// Perform one iteration of sandpile toppling - process ALL unstable cells simultaneously
int local_sandpile_iteration(SandpileData *data) {
    int changed = 0;
    
    // Process only internal cells (not ghost cells) - but use original algorithm logic
    for (int i = 1; i <= data->local_rows; i++) {
        for (int j = 1; j <= data->local_cols; j++) {
            if (data->grid[i][j] >= 4) {
                // Use same logic as original: integer division and modulo
                int dist = data->grid[i][j] >> 2;
                data->grid[i][j] &= 3;
                
                // Distribute to neighbors (including ghost cells)
                data->grid[i-1][j] += dist;  
                data->grid[i+1][j] += dist;  
                data->grid[i][j-1] += dist;  
                data->grid[i][j+1] += dist;  
                changed = 1;
            }
        }
    }
    
    return changed;
}

// Main sandpile simulation with proper boundary handling
void run_sandpile_simulation(SandpileData *data) {
    int iteration = 0;
    int global_changed = 1;
    
    while (global_changed) {
        iteration++;
        
        // Perform local sandpile iteration (may update ghost cells)
        int local_changed = local_sandpile_iteration(data);
        
        // Collect contributions from ghost cells and send back to neighbors
        collect_boundary_contributions(data);
        
        // Check if any process had changes
        MPI_Allreduce(&local_changed, &global_changed, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
    }
    if (data->rank == 0) {
        printf("Sandpile stabilized after %d iterations\n", iteration - 1);
    }
}

// Gather and print final result
void print_final_grid(SandpileData *data) {

    // Create arrays to store sizes and displacements for gatherv
    int *recvcounts = NULL;
    int *displs = NULL;
    int *global_grid = NULL;
    
    if (data->rank == 0) {
        recvcounts = (int*)malloc(data->size * sizeof(int));
        displs = (int*)malloc(data->size * sizeof(int));
        global_grid = (int*)malloc(data->global_rows * data->global_cols * sizeof(int));
        
        // Calculate receive counts and displacements for each process
        for (int p = 0; p < data->size; p++) {
            int p_row = p / data->proc_cols;
            int p_col = p % data->proc_cols;
            
            // Calculate local grid size for process p
            int base_rows = data->global_rows / data->proc_rows;
            int base_cols = data->global_cols / data->proc_cols;
            int extra_rows = data->global_rows % data->proc_rows;
            int extra_cols = data->global_cols % data->proc_cols;
            
            int p_local_rows = (p_row < extra_rows) ? base_rows + 1 : base_rows;
            int p_local_cols = (p_col < extra_cols) ? base_cols + 1 : base_cols;
            
            recvcounts[p] = p_local_rows * p_local_cols;
            displs[p] = (p == 0) ? 0 : displs[p-1] + recvcounts[p-1];
        }
    }
    
    // Pack local interior data
    int local_size = data->local_rows * data->local_cols;
    int *local_data = (int*)malloc(local_size * sizeof(int));
    
    for (int i = 0; i < data->local_rows; i++) {
        for (int j = 0; j < data->local_cols; j++) {
            local_data[i * data->local_cols + j] = data->grid[i + 1][j + 1];
        }
    }
    
    // Gather all local data to rank 0
    MPI_Gatherv(local_data, local_size, MPI_INT,
                global_grid, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (data->rank == 0) {
        printf("\nFinal stable configuration:\n");
        
        // Reconstruct global grid from gathered data
        int **reconstructed = (int**)malloc(data->global_rows * sizeof(int*));
        for (int i = 0; i < data->global_rows; i++) {
            reconstructed[i] = (int*)malloc(data->global_cols * sizeof(int));
        }
        
        // Fill reconstructed grid
        int offset = 0;
        for (int p = 0; p < data->size; p++) {
            int p_row = p / data->proc_cols;
            int p_col = p % data->proc_cols;
            
            // Calculate process p's domain
            int base_rows = data->global_rows / data->proc_rows;
            int base_cols = data->global_cols / data->proc_cols;
            int extra_rows = data->global_rows % data->proc_rows;
            int extra_cols = data->global_cols % data->proc_cols;
            
            int p_local_rows = (p_row < extra_rows) ? base_rows + 1 : base_rows;
            int p_local_cols = (p_col < extra_cols) ? base_cols + 1 : base_cols;
            
            int p_start_row = (p_row < extra_rows) ? p_row * (base_rows + 1) : 
                              extra_rows * (base_rows + 1) + (p_row - extra_rows) * base_rows;
            int p_start_col = (p_col < extra_cols) ? p_col * (base_cols + 1) :
                              extra_cols * (base_cols + 1) + (p_col - extra_cols) * base_cols;
            
            // Copy data from process p
            for (int i = 0; i < p_local_rows; i++) {
                for (int j = 0; j < p_local_cols; j++) {
                    reconstructed[p_start_row + i][p_start_col + j] = 
                        global_grid[offset + i * p_local_cols + j];
                }
            }
            offset += p_local_rows * p_local_cols;
        }
        
        // Print reconstructed grid
        for (int i = 0; i < data->global_rows; i++) {
            for (int j = 0; j < data->global_cols; j++) {
                printf("%d ", reconstructed[i][j]);
            }
            printf("\n");
        }
        
        // Cleanup
        for (int i = 0; i < data->global_rows; i++) {
            free(reconstructed[i]);
        }
        free(reconstructed);
        free(recvcounts);
        free(displs);
        free(global_grid);
    }
    
    free(local_data);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    SandpileData data;
    MPI_Comm_rank(MPI_COMM_WORLD, &data.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &data.size);
    
    // Default parameters
    data.global_rows = 60;
    data.global_cols = 30;
    int center_value = 12121;
    int default_value = 624;
    
    if (argc == 5) {
        data.global_rows = atoi(argv[1]);
        data.global_cols = atoi(argv[2]);
        center_value = atoi(argv[3]);
        default_value = atoi(argv[4]);
    }
    
    setup_domain_decomposition(&data);
    
    if (data.rank == 0) {
        printf("Running MPI sandpile simulation on %d processes\n", data.size);
        printf("Global grid: %dx%d\n", data.global_rows, data.global_cols);
        printf("Process grid: %dx%d\n", data.proc_rows, data.proc_cols);
    }
    

    initialize_sandpile(&data, center_value, default_value);
    
    double start_time = MPI_Wtime();
    run_sandpile_simulation(&data);
    double end_time = MPI_Wtime();
    
    if (data.rank == 0) {
        printf("Simulation completed in %.4f seconds\n", end_time - start_time);
    }

    print_final_grid(&data);
    free_grid(data.grid, data.local_rows);
    
    MPI_Finalize();
    return 0;
}