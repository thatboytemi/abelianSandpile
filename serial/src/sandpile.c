#include "../include/sandpile.h"
#include "../include/grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

double time_sync = 0.0;
double time_async = 0.0;

int sync_new_tile(int x, int y, Grid *current, Grid *next) {

    // update the current tile synchronously using tiles surrounding (x, y)
    next->sandpile[y][x] = current->sandpile[y][x] % 4
    + current->sandpile[y-1][x] / 4
    + current->sandpile[y+1][x] / 4
    + current->sandpile[y][x-1] / 4
    + current->sandpile[y][x+1] / 4;

    if (next->sandpile[y][x] > 3) {
        return 1; // Count so that we know when it is stable
    }

    return 0; 
}

int async_new_tile(int x, int y, Grid *grid) {
    // Only one grid, update each surrounding block
    if (grid->sandpile[y][x] < 4) return 0; // If tile stable no need to update neighbours
    unsigned long int div4 = grid->sandpile[y][x] / 4; 
    grid_add(grid, y, x-1, div4);
    grid_add(grid, y, x+1, div4);
    grid_add(grid, y-1, x, div4);
    grid_add(grid, y+1, x, div4);
    // Remainder stays in the current tile
    grid->sandpile[y][x] %= 4;
    return 1; // Count so that we know when it is stable
}

void topple_asynch(Grid *grid) {

    // One grid
    int rows = grid->rows;
    int cols = grid->cols;
    int stable;

    clock_t start = clock();
    while (true) {
        stable = 0;
        for (int y = 1; y <= rows; y++) {
            
            for (int x = 1; x <= cols; x++) {
                stable += async_new_tile(x, y, grid);   
            }
        }
        // printf("Stable: %d\n", stable); // Debug print
        if (stable == 0) {
            break; // If no tiles unstable, we are stable
        }
        // Print the current grid state for debugging
        // grid_print(grid); // Uncomment to see the grid state after each iteration
        // printf("\n");

    }
    clock_t end = clock();
    time_async = (double)(end - start) / CLOCKS_PER_SEC;
}

void topple_sync(Grid *current, Grid *next) {
    int rows = current->rows;
    int cols = current->cols;
    int stable;

    clock_t start = clock();
    while (true) {
        stable = 0;
        for (int y = 1; y <= rows; y++) {
            for (int x = 1; x <= cols; x++) {
                stable += sync_new_tile(x, y, current, next);   
            }
        }
        // printf("Stable: %d\n", stable); // Debug print
        if (stable == 0) break; // If no tiles unstable, we are stable

        // Print the current grid state for debugging
        // printf("\n");
        // grid_print(current); // Uncomment to see the grid state after each iteration
        // printf("\n");
        // grid_print(next); // Uncomment to see the next grid state after each iteration
        // printf("\n");
        // Swap grids
        grid_swap(&current, &next);
    }
    
    clock_t end = clock();
    time_sync = (double)(end - start) / CLOCKS_PER_SEC;
}
    
