#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <time.h>

static void printUsage (char *cmdName);
int checkIfProperlySorted(const int *array, int size, int direction);
static double get_delta_time(void);

int main(int argc, char *argv[]) {

    int rank, nProc;                               /* rank of the process and number of processes */

    MPI_Init (&argc, &argv);                                    /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);               /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &nProc);         /* get number of processes */

    // Check if the number of processes is a power of 2
    if ((nProc & (nProc - 1)) != 0) {
        if (rank == 0) {
            fprintf(stderr, "Number of processes must be a power of 2\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int opt;                                       /* selected option */
    char* fName;                                   /* file name */
    int nThreads = 8;                              /* number of threads (initialized to 8 by default) */
    int direction = 0;                             /* sort direction (initialized to 0 by default) */

    opterr = 0;
    do
    { switch ((opt = getopt (argc, argv, "t:d:h")))
        { case 't': /* number of threads */
                if (atoi (optarg) != 1 && atoi (optarg) != 2 && atoi (optarg) != 4 && atoi (optarg) != 8)
                { fprintf (stderr, "%s: invalid number of threads (must be 1 or 2 or 4 or 8)\n", basename (argv[0]));
                    printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
                }
                nThreads = (int) atoi (optarg);
                break;
            case 'd': /* sort direction */
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
    printf ("Number of threads = %d\n", nThreads);
    printf ("sort direction = %d\n", direction);

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
                     "  -t      --- number of threads (default = 8)\n"
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