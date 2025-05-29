#include "../include/grid.h"

#ifndef SANDPILE_H 
#define SANDPILE_H

void async_new_tile(int x, int y, Grid *grid);
void topple_asynch(Grid *grid);

extern double time_async; // Asynchronous time
extern double time_sync; // Synchronous time

#endif