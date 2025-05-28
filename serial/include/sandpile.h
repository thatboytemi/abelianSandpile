#include "../include/grid.h"

#ifndef SANDPILE_H 
#define SANDPILE_H

int sync_new_tile(int x, int y, Grid *current, Grid *next);
int async_new_tile(int x, int y, Grid *grid);
void topple(Grid *current, Grid *next);

#endif