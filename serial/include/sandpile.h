#include "../include/grid.h"

#ifndef SANDPILE_H 
#define SANDPILE_H

int sync_new_tile(int x, int y, Grid *current, Grid *next);
int async_new_tile(int x, int y, Grid *grid);
void topple_sync(Grid *current, Grid *next);
void topple_asynch(Grid *grid);

extern double time_async; // Asynchronous time
extern double time_sync; // Synchronous time

#endif