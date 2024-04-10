//
// Created by diogopaiva21 on 11-03-2024.
//

#include "bitonicSort.h"

/**
 *  \brief Swap two integers.
 *
 *  \param a pointer to the first integer
 *  \param b pointer to the second integer
 */
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/**
 *  \brief Merge two sub-arrays in bitonic order.
 *
 *  \param arr pointer to the array
 *  \param n size of the array
 *  \param direction 0 for ascending order, 1 for descending order
 */
void bitonicMerge(int arr[], int n, int direction) {
    int i, j, k;

    for (k = n/2; k <= n; k = 2 * k) {
        for (j = k/2; j > 0; j = j/2) {
            // Iterate through the array
            for (i = 0; i < n; i++) {
                int ixj = i ^ j;

                if ((ixj) > i) {
                    if ((i & k) == 0 && ((direction == 0 && arr[i] > arr[ixj]) || (direction == 1 && arr[i] < arr[ixj]))) {
                        swap(&arr[i], &arr[ixj]);
                    }
                    if ((i & k) != 0 && ((direction == 0 && arr[i] < arr[ixj]) || (direction == 1 && arr[i] > arr[ixj]))) {
                        swap(&arr[i], &arr[ixj]);
                    }
                }
            }
        }
    }
}

/**
 *  \brief Sort an array in bitonic order.
 *
 *  \param arr pointer to the array
 *  \param n size of the array
 *  \param direction 0 for ascending order, 1 for descending order
 */
void bitonicSort(int arr[], int n, int direction) {
    int i, j, k;

    for (k = 2; k <= n; k = 2 * k) {
        for (j = k/2; j > 0; j = j/2) {
            // Iterate through the array
            for (i = 0; i < n; i++) {
                int ixj = i ^ j;

                if ((ixj) > i) {
                    if ((i & k) == 0 && ((direction == 0 && arr[i] > arr[ixj]) || (direction == 1 && arr[i] < arr[ixj]))) {
                        swap(&arr[i], &arr[ixj]);
                    }
                    if ((i & k) != 0 && ((direction == 0 && arr[i] < arr[ixj]) || (direction == 1 && arr[i] > arr[ixj]))) {
                        swap(&arr[i], &arr[ixj]);
                    }
                }
            }
        }
    }

}