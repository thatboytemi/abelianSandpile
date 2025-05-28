#include "../include/grid.h"
#include <stdlib.h>
#include <stdio.h>

Grid* grid_create(int rows, int cols, unsigned long int centre, unsigned long int allVal) {
    Grid *grid = (Grid*)malloc(sizeof(Grid));
    grid->rows = rows;
    grid->cols = cols;

    grid->sandpile = (unsigned long int**)malloc(rows * sizeof(unsigned long int*));
    for (int i = 0; i < rows; i++) {
        grid->sandpile[i] = (unsigned long int*)malloc(cols * sizeof(unsigned long int));
        // Initialize to 0
        for (int j = 0; j < cols; j++) {
            grid->sandpile[i][j] = allVal;
        }
    }
    grid->sandpile[rows/2][cols/2] = centre; 

    return grid;
}

void grid_add(Grid *grid, int y, int x, int value) {
    if (y >= 0 || y < grid->rows || x >= 0 || x < grid->cols) {
        grid->sandpile[y][x] += value;
    }
}

void grid_swap(Grid **current, Grid **next) {
    // Switch pointers of current and next grids
    Grid *temp = *current;
    *current = *next;
    *next = temp;
    printf("Swapped grids.\n");
}

void grid_print(Grid *grid) {
    int rows = grid->rows;
    int cols =  grid->cols;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%ld ", grid->sandpile[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
