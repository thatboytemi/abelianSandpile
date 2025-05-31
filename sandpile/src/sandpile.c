#include "../include/sandpile.h"
#include "../include/grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

double time_async = 0.0;
#define THRESHOLD 4
// Flag to indicate if any tile was unstable
int stable = 0;

void async_new_tile(int x, int y, Grid *grid) {
    // Only one grid, update each surrounding block
    if (grid->sandpile[y][x] >= THRESHOLD) {

    unsigned long int div4 = grid->sandpile[y][x] / 4; 
    // Update the surrounding tiles
    grid->sandpile[y][x - 1] += div4; // Left
    grid->sandpile[y][x + 1] += div4; // Right
    grid->sandpile[y - 1][x] += div4; // Up
    grid->sandpile[y + 1][x] += div4; // Down

    // Remainder stays in the current tile
    grid->sandpile[y][x] %= 4;    
    // Set flag to indicate at least one tile was unstable
    if (!stable) stable = 1; 
    }
}


void topple_asynch(Grid *grid) {

    // One grid
    int rows = grid->rows;
    int cols = grid->cols;

    clock_t start = clock();
    while (true) {
        stable = 0; // Reset stable flag for each iteration
        for (int y = 1; y <= rows; y++) {
            for (int x = 1; x <= cols; x++) {
                async_new_tile(x, y, grid);   
            }
        }
        if (stable == 0) {
            break; // If no tiles unstable, we are stable
        }
    }
    clock_t end = clock();
    time_async = (double)(end - start) / CLOCKS_PER_SEC;
}

    
