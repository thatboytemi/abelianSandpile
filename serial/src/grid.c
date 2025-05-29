#include "../include/grid.h"
#include <stdlib.h>
#include <stdio.h>

Grid* grid_create(int rows, int cols, unsigned long int centre, unsigned long int allVal) {
    Grid *grid = (Grid*)malloc(sizeof(Grid));
    grid->rows = rows;
    grid->cols = cols;
    
    grid->sandpile = (unsigned long int**)malloc((rows + 2) * sizeof(unsigned long int*));
    for (int i = 0; i <= rows + 1; i++) {
        grid->sandpile[i] = (unsigned long int*)malloc((cols + 2) * sizeof(unsigned long int));
        for (int j = 0; j <= cols + 1; j++) {
            grid->sandpile[i][j] = allVal;
        }
    }

    grid->sandpile[rows/2 + 1][cols/2 + 1] = centre; 

    return grid;
}

void add_padding(int rows, int cols, Grid *grid) {
    // Add padding to the grid to avoid boundary checks
    for (int i = 0; i <= rows + 1; i++) {
        grid->sandpile[i][0] = 0; // Left padding
        grid->sandpile[i][cols + 1] = 0; // Right padding
    }
    for (int j = 0; j <= cols + 1; j++) {
        grid->sandpile[0][j] = 0; // Top padding
        grid->sandpile[rows + 1][j] = 0; // Bottom padding
    }
}

void grid_print(Grid *grid) {
    int rows = grid->rows;
    int cols =  grid->cols;

    for (int i = 1; i <= rows; i++) {
        for (int j = 1; j <= cols; j++) {
            printf("%ld ", grid->sandpile[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void grid_free(Grid *grid) {
    if (grid) {
        for (int i = 0; i < grid->rows+1; i++) {
            free(grid->sandpile[i]);
        }
        free(grid->sandpile);
        free(grid);
    }
}
