//
// Created by diogopaiva21 on 13-03-2024.
//

#include "fileReader.h"
#include <stdio.h>
#include <stdlib.h>

/**
 *  \brief Read the file and store the array in the memory
 *
 *  \param fName file name
 *  \param arr pointer to the array
 *
 *  \return size of the array
 */
int readToArr(char *fName, int **arr) {

    FILE *file = fopen(fName, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: file not found\n");
        exit(EXIT_FAILURE);
    }

    // Read from the file the first int
    int size;
    if(fread(&size, sizeof(int), 1, file) != 1) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }

    *arr = (int *)malloc(size * sizeof(int));
    if (*arr == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Read from the file the array of n ints
    if(fread(*arr, sizeof(int), size, file) != size) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }

    fclose(file);

    return size;
}