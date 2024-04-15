#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void merge(int arr[], int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temporary arrays
    int *L = malloc(n1 * sizeof(int));
    int *R = malloc(n2 * sizeof(int));

    // Copy data to temporary arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Merge the temporary arrays back into arr[l..r]
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    // Copy the remaining elements of L[], if any
    while (i < n1) {
        arr[k++] = L[i++];
    }

    // Copy the remaining elements of R[], if any
    while (j < n2) {
        arr[k++] = R[j++];
    }

    free(L);
    free(R);
}

void mergeSort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int n;
    double startTime, endTime;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // printf("test");
    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <array_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    if (n <= 0) {
        if (rank == 0) {
            printf("Invalid array size\n");
        }
        MPI_Finalize();
        return 1;
    }

    int *arr = NULL;
    int local_size = n / size;
    int *local_arr = (int *)malloc(local_size * sizeof(int));

    if (rank == 0) {
        arr = (int *)malloc(n * sizeof(int));
        FILE *input_file = fopen("array1.txt", "r");
        if (input_file == NULL) {
            printf("Error opening file: array1.txt\n");
            MPI_Finalize();
            return 1;
        }
        for (int i = 0; i < n; i++) {
            if (fscanf(input_file, "%d,", &arr[i]) != 1) {
                printf("Error reading from file\n");
                fclose(input_file);
                MPI_Finalize();
                return 1;
            }
        }
        fclose(input_file);
    }

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize all processes

    int num_trials = 5;
    double total_time = 0.0;

    for (int trial = 0; trial < num_trials; trial++) {
        MPI_Barrier(MPI_COMM_WORLD); // Synchronize before each trial

        startTime = MPI_Wtime(); // Start timing

        MPI_Scatter(arr, local_size, MPI_INT, local_arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        mergeSort(local_arr, 0, local_size - 1);

        MPI_Gather(local_arr, local_size, MPI_INT, arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        endTime = MPI_Wtime(); // Stop timing

        if (rank == 0) {
            double trial_time = (endTime - startTime) * 1000.0; // Convert to milliseconds
            // printf("Trial %d: Sorting time - %.2f milliseconds\n", trial + 1, trial_time);
            total_time += trial_time;
        }
    }

    free(local_arr);

    if (rank == 0) {
        printf("Average sorting time over %d trials: %.2f milliseconds\n", num_trials, total_time / num_trials);

        FILE *output_file = fopen("sorted.txt", "w");
        if (output_file == NULL) {
            printf("Error opening file: sorted.txt\n");
            free(arr);
            MPI_Finalize();
            return 1;
        }

        for (int i = 0; i < n; i++) {
            fprintf(output_file, "%d, ", arr[i]);
        }

        fclose(output_file);
        free(arr);
    }

    MPI_Finalize();
    return 0;
}
