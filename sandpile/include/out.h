#ifndef OUT_H 
#define OUT_H

#include "grid.h"
#include <stdio.h>
#include <stdlib.h>

void write_results(const char *filename, const char *version, int num_Threads, int rows, int cols, unsigned long int centre, unsigned long int allVal, double async_time);
void visualize_grid_as_image(Grid* grid, const char *filename);
void gridWrite(FILE *file, int** sandpile, int i, int j, int value);
void vis_grid(int** sandpile, const char *filename, int rows, int cols, bool mpi);
#endif