#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void merge(int arr[], int l, int m, int r, int temp[]) {
    int i = l, j = m + 1, k = l;

    // Merge the two halves into temp array
    while (i <= m && j <= r) {
        if (arr[i] <= arr[j])
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }

    // Copy the remaining elements from the left half
    while (i <= m)
        temp[k++] = arr[i++];

    // Copy the remaining elements from the right half
    while (j <= r)
        temp[k++] = arr[j++];

    // Copy the merged elements back to the original array
    for (i = l; i <= r; i++)
        arr[i] = temp[i];
}

void mergesort_serial(int arr[], int l, int r, int temp[]) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergesort_serial(arr, l, m, temp);
        mergesort_serial(arr, m + 1, r, temp);
        merge(arr, l, m, r, temp);
    }
}

void mergesort_parallel_omp(int arr[], int l, int r, int temp[], int threads) {
    if (threads < 1) {
        // Perform serial merge sort if threads < 1
        mergesort_serial(arr, l, r, temp);
        return;
    }

    if (l < r) {
        int m = l + (r - l) / 2;

        // Use OpenMP sections for parallel recursive calls
#pragma omp parallel sections
        {
#pragma omp section
            mergesort_parallel_omp(arr, l, m, temp, threads / 2); // Left half

#pragma omp section
            mergesort_parallel_omp(arr, m + 1, r, temp, threads - threads / 2); // Right half
        }

        // Merge the two sorted halves
        merge(arr, l, m, r, temp);
    }
}

int main(int argc, char *argv[]) {
    int n;
    int num_threads = 1; // Default number of threads

    if (argc != 3) {
        printf("Usage: %s <array_size> <num_threads>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    if (n <= 0) {
        printf("Invalid array size\n");
        return 1;
    }

    num_threads = atoi(argv[2]);
    if (num_threads <= 0) {
        printf("Invalid number of threads\n");
        return 1;
    }

    int *arr = malloc(n * sizeof(int));
    int *temp = malloc(n * sizeof(int));
    if (arr == NULL || temp == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    FILE *input_file = fopen("array1.txt", "r");
    if (input_file == NULL) {
        printf("Error opening file: array1.txt\n");
        free(arr);
        free(temp);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        if (fscanf(input_file, "%d, ", &arr[i]) != 1) {
            printf("Error reading from file\n");
            fclose(input_file);
            free(arr);
            free(temp);
            return 1;
        }
    }

    fclose(input_file);

    omp_set_num_threads(num_threads);

    double total_time = 0;
    int num_trials = 5;

    for (int t = 0; t < num_trials; t++) {
        // Create a temporary array for sorting in each trial
        for (int i = 0; i < n; i++) {
            temp[i] = arr[i]; // Copy original array to temp array
        }

        double start = omp_get_wtime();
        mergesort_parallel_omp(temp, 0, n - 1, arr, num_threads); // Perform merge sort
        double end = omp_get_wtime();

        double elapsed_time = (end - start) * 1000.0; // Convert to milliseconds
        total_time += elapsed_time;

        // printf("Trial %d: Sorting time - %.2f milliseconds\n", t + 1, elapsed_time);
    }

    // Calculate average time
    double average_time = total_time / num_trials;
    printf("\nAverage sorting time over %d trials: %.2f milliseconds\n", num_trials, average_time);

    FILE *output_file = fopen("sorted.txt", "w");
    if (output_file == NULL) {
        printf("Error opening file: sorted.txt\n");
        free(arr);
        free(temp);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        fprintf(output_file, "%d, ", arr[i]);
    }

    fclose(output_file);
    // Free dynamically allocated memory
    free(arr);
    free(temp);

    return 0;
}
