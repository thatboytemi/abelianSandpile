#include "serial/include/grid.h"
#include "serial/include/sandpile.h"
#include "serial/include/out.h"
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
    add_padding(rows, cols, current);
    Grid* next = grid_create(rows, cols, centre, allVal);
    add_padding(rows, cols, next);
    topple_sync(current, next);

    Grid* sandpile = grid_create(rows, cols, centre, allVal);
    add_padding(rows, cols, sandpile);
    topple_asynch(sandpile);

    visualize_grid_as_image(sandpile, "output_2.ppm");

    bool gridEqual = check_equal(next, sandpile);
    write_results("output.txt", rows, cols, centre, allVal, gridEqual, time_async, time_sync);

    // Free allocated memory
    grid_free(current);
    grid_free(next);
    grid_free(sandpile);

    return 0;
}