#include "serial/include/grid.h"
#include "serial/include/sandpile.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Define initialisation params
    int rows = 60;
    int cols = 30;
    int centre = 12121;
    int allVal = 624;

    // Override defaults if arguments are provided
    if (argc == 5) {
        rows = atoi(argv[1]);
        cols = atoi(argv[2]);
        centre = atoi(argv[3]);
        allVal = atoi(argv[4]);
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [rows cols centre allVal]\n", argv[0]);
        return 1;
    }

    Grid* current = grid_create(rows, cols, centre, allVal);
    Grid* next = grid_create(rows, cols, centre, allVal);
    topple(current, next);
    // Free allocated memory
    for (int i = 0; i < rows; i++) {
        free(current->sandpile[i]);
        free(next->sandpile[i]);
    }
    free(current->sandpile);
    free(next->sandpile);
    free(current);
    free(next);
    printf("Finished processing sandpile.\n");
    return 0;
}