#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "reader/fileReader.h"
#include "sort/bitonicSort.h"

struct Setting {
    int direction;
    int size;
};

static void printUsage (char *cmdName);
int checkIfProperlySorted(const int *array, int size, int direction);
static double get_delta_time(void);

int main(int argc, char *argv[]) {

    int rank, nProc, nowProc;                               /* rank of the process, number of processes and present number of processes */
    struct Setting settings;                               /* setting struct */
    int *array = NULL;                               /* array to be sorted */

    MPI_Init (&argc, &argv);                                    /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);               /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &nProc);         /* get number of processes */

    MPI_Group nowGroup, nextGroup;                                 /* Group of processes */
    MPI_Comm nowComm, nextComm;                                       /* Communication group */

    int Group[8];                                           /* Group of processes */

    // Check if the number of processes is a power of 2
    if ((nProc & (nProc - 1)) != 0) {
        if (rank == 0) {
            fprintf(stderr, "Number of processes must be a power of 2\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Check if the number of processes is greater or equal to 1 and less or equal to 8
    if (nProc < 1 || nProc > 8) {
        if (rank == 0) {
            fprintf(stderr, "Number of processes must be greater or equal to 1 and less or equal to 8\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (rank == 0){

        // Parse command line arguments
        int opt;                                       /* selected option */
        char* fName;                                   /* file name */
        int direction = 0;                             /* sort direction (initialized to 0 by default) */

        opterr = 0;
        do
        { switch ((opt = getopt (argc, argv, "d:h")))
            {   case 'd': /* sort direction */
                    if (atoi(optarg) != 0 && atoi(optarg) != 1) {
                        fprintf(stderr, "%s: invalid sort direction\n", basename(argv[0]));
                        printUsage(basename(argv[0]));
                        exit(EXIT_FAILURE);
                    }
                    direction = (int) atoi (optarg);
                    break;
                case 'h': /* help mode */
                    printUsage (basename (argv[0]));
                    return EXIT_SUCCESS;
                case '?': /* invalid option */
                    fprintf (stderr, "%s: invalid option\n", basename (argv[0]));
                    printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
                case -1:  break;
            }
        } while (opt != -1);
        if (argc == 1)
        { fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
            printUsage (basename (argv[0]));
            return EXIT_FAILURE;
        }

        if (optind >= argc) {
            fprintf(stderr, "%s: no file name provided\n", basename(argv[0]));
            printUsage(basename(argv[0]));
            exit(EXIT_FAILURE);
        }
        fName = argv[optind];

        printf ("File name = %s\n", fName);
        printf ("sort direction = %d\n", direction);

        // Get Array from file
        int size = readToArr(fName, &array);

        // Create a setting struct
        settings.direction = direction;
        settings.size = size;

        // Send the setting struct to all processes
        MPI_Bcast(&settings, 1, MPI_2INT, 0, MPI_COMM_WORLD);
    } else {
        // Receive the setting struct from the root process
        MPI_Bcast(&settings, 1, MPI_2INT, 0, MPI_COMM_WORLD);
    }

    // Allocate memory for the recv buffer
    int *recvBuffer = (int *) malloc(settings.size * sizeof(int));

    // Calculate how many iterations to sort the array
    int nIterations = (int) log2(nProc);

    // SubArray size
    int subArraySize = 0;

    // Create Communication Groups
    nowComm = MPI_COMM_WORLD;
    MPI_Comm_group (nowComm, &nowGroup);

    // Number of current processes
    nowProc = nProc;

    // Create the group of processes
    for (int i = 0; i < nProc; i++) {
        Group[i] = i;
    }

    if(rank == 0) {
        // Start the timer
        get_delta_time();
    }

    // Iterations
    for (int i = 0; i <= nIterations; i++) {

        // If not the first iteration
        if (i != 0) {

            // Create the next communication group
            MPI_Group_incl(nowGroup, nowProc, Group, &nextGroup);

            // Create the next communicator
            MPI_Comm_create(nowComm, nextGroup, &nextComm);

            // Update
            nowComm = nextComm;
            nowGroup = nextGroup;

            // Finish if rank of the process is bigger than the now number of processes
            if (rank >= nowProc) {
                free(recvBuffer);
                MPI_Finalize();
                return EXIT_SUCCESS;
            }
        }

        // Update the number of current processes
        MPI_Comm_size (nowComm, &nProc);

        // Calculate the size of the subArray
        subArraySize = settings.size / nowProc;

        // Scatter the array
        MPI_Scatter(array, subArraySize, MPI_INT, recvBuffer, subArraySize, MPI_INT, 0, nowComm);

        // Sort the subArray
        if(i == 0) {
            bitonicSort(recvBuffer, subArraySize, settings.direction);
        } else {
            bitonicMerge(recvBuffer, subArraySize, settings.direction);
        }

        // Gather the sorted subArray
        MPI_Gather(recvBuffer, subArraySize, MPI_INT, array, subArraySize, MPI_INT, 0, nowComm);

        // Update the number of current processes
        nowProc = nProc / 2;
    }

    if (rank == 0) {
        // Check if the array is properly sorted
        if (checkIfProperlySorted(array, settings.size, settings.direction)) {
            printf("Array is properly sorted\n");
        } else {
            printf("Array is not properly sorted\n");
        }

        // Print elapsed time
        printf("Elapsed time: %f\n", get_delta_time());
    }

    free(recvBuffer);
    free(array);
    MPI_Finalize();
    return EXIT_SUCCESS;
}

/**
 *  \brief Print command usage.
 *
 *  A message specifying how the program should be called is printed.
 *
 *  \param cmdName string with the name of the command
 */

static void printUsage (char *cmdName)
{
    fprintf (stderr, "\nSynopsis: %s OPTIONS <FILE>\n"
                     "  OPTIONS:\n"
                     "  -d      --- direction (0 = ascending || 1 = descending) (default = 0) \n"
                     "  -h      --- print this help\n", cmdName);
}

/**
 *  \brief Check if array is properly sorted.
 *
 *  \param array array to be checked
 *  \param size size of the array
 *  \param direction sort direction (0 = ascending || 1 = descending)
 *
 *  \return 1 if array is properly sorted, 0 otherwise
 */

int checkIfProperlySorted(const int *array, int size, int direction) {
    int i;
    if (direction == 0) {
        for (i = 0; i < size - 1; i++) {
            if (array[i] > array[i + 1]) {
                return 0;
            }
        }
    } else {
        for (i = 0; i < size - 1; i++) {
            if (array[i] < array[i + 1]) {
                return 0;
            }
        }
    }
    return 1;
}

/**
 *  \brief Get the process time that has elapsed since last call of this time.
 *
 *  \return process elapsed time
 */
static double get_delta_time(void) {
    static struct timespec t0, t1;

    t0 = t1;
    if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec) ;
}