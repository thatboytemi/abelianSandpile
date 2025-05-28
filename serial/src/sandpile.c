#include "../include/sandpile.h"
#include "../include/grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


int sync_new_tile(int x, int y, Grid *current, Grid *next) {

    // update the current tile synchronously using tiles surrounding (x, y)
    next->sandpile[y][x] = current->sandpile[y][x] % 4;

    // Update current tile using surrounding tiles
    if (y > 0) next->sandpile[y][x] += current->sandpile[y-1][x] / 4;
    if (y < current->rows - 1) next->sandpile[y][x] += current->sandpile[y+1][x] / 4;
    if (x > 0) next->sandpile[y][x] += current->sandpile[y][x-1] / 4;
    if (x < current->cols - 1) next->sandpile[y][x] += current->sandpile[y][x+1] / 4;

    if (next->sandpile[y][x] > 3) {
        return 1; // Count so that we know when it is stable
    }

    return 0; 
}

int async_new_tile(int x, int y, Grid *grid) {
    // Only one grid, update each surrounding block
    if (grid->sandpile[y][x] < 4) return 0; 
    unsigned long int div4 = grid->sandpile[y][x] / 4; 
    grid_add(grid, y, x-1, div4);
    grid_add(grid, y, x+1, div4);
    grid_add(grid, y-1, x, div4);
    grid_add(grid, y+1, x, div4);
    // Remainder stays in the current tile
    grid->sandpile[y][x] %= 4;
    return 1; // Count so that we know when it is stable
}

void topple(Grid *current, Grid *next) {
    int rows = current->rows;
    int cols = current->cols;
    int stable;

    while (true) {
        stable = 0;
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                stable += sync_new_tile(x, y, current, next);   
            }
        }
        // print both grids for debugging
        printf("T:\n");
        grid_print(current);
        printf("\nT+1:\n");
        grid_print(next);

        printf("%d\n", stable);
        if (stable == 0) {
            break; // If no tiles unstable, we are stable
            printf("Stable state:\n");
            grid_print(next);
        }
        
        // Swap grids
        grid_swap(&current, &next);

    }
}
    
