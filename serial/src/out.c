// Visualize the grid as an image
// Write results to a file

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/grid.h"

void write_results(const char *filename, int rows, int cols,
    unsigned long int centre, unsigned long int allVal, bool gridEqual, double async_time, double sync_time) {

    FILE *file = fopen(filename, "a");
    if (!file) {
        perror("Failed to open file for writing");
        printf("Error opening file: %s\n", filename); // Debug print
        return;
    }

    fprintf(file, "Rows: %d\n", rows);
    fprintf(file, "Cols: %d\n", cols);
    fprintf(file, "Centre: %lu\n", centre);
    fprintf(file, "AllVal: %lu\n", allVal);
    fprintf(file, "Grids equal: %s\n", gridEqual ? "EQUAL" : "NOT EQUAL");
    fprintf(file, "Synchronous time: %lf seconds\n", sync_time);
    fprintf(file, "Asynchronous time: %lf seconds\n", async_time);
    fprintf(file, "----------------------------------------\n");
    
    fclose(file);
    printf("Data written successfully to %s\n", filename); // Debug print
}

// Function to visualize the grid as a PPM image
void visualize_grid_as_image(Grid *grid, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing.");
        return;
    }

    // write the PPM header
    fprintf(file, "P3\n"); // P3 indicates plain-text PPM format
    fprintf(file, "%d %d\n", grid->cols, grid->rows); // Width and height
    fprintf(file, "255\n"); // Maximum color value (0-255)

    unsigned long int value = 0;
    // Write pixel data
    for (int i = 1; i <= grid->rows; i++) {
        for (int j = 1; j <= grid->cols; j++) {
            value = grid->sandpile[i][j];
            if (value == 0) {
                fprintf(file, "0 0 0 ");

            } else if (value == 1) {
                fprintf(file, "0 255 0 ");

            } else if (value == 2) {
                fprintf(file, "0 0 255 ");

            } else if (value == 3) {
                // Light gray for higher values
                fprintf(file, "255 0 0 ");
            }

        fprintf(file, "\n");
        }
    }

    fclose(file);
    printf("Image saved to %s\n", filename);
}
