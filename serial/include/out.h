#ifndef OUT_H 
#define OUT_H

#include "grid.h"

void write_results(const char *filename, int rows, int cols, unsigned long int centre, unsigned long int allVal, bool gridEqual, double sync_time, double async_time);
void visualize_grid_as_image(Grid *grid, const char *filename);

#endif