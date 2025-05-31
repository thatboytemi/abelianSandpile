// 2d grid representing the sandpile
#include <stdbool.h>
// guards prevent multiple inclusions
#ifndef GRID_H 
#define GRID_H

typedef struct Grid {
    int rows;
    int cols;
    unsigned long int **sandpile; // Double pointer for 2D array
} Grid;

// grid_create uses malloc so must return memory address assigned
Grid* grid_create(int rows, int cols, unsigned long int centre, unsigned long int allVal); 
void grid_free(Grid *grid);
// For reading cell values from grid
// int grid_get(const Grid *grid, int x, int y);
// // For writing to grid
void grid_add(Grid *grid, int x, int y, int value);
void grid_swap(Grid **current, Grid **next);
void grid_print(Grid *grid);
bool check_equal(Grid *grid1, Grid *grid2);
void add_padding(int rows, int cols, Grid *grid);

#endif