#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

void merge(int arr[], int temp[], int l, int m, int r) {
    int i = l;    // Initial index for left subarray
    int j = m + 1; // Initial index for right subarray
    int k = l;    // Initial index for merged subarray

    // Merge the two halves into temp array
    while (i <= m && j <= r) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    // Copy the remaining elements of left subarray, if any
    while (i <= m) {
        temp[k++] = arr[i++];
    }

    // Copy the remaining elements of right subarray, if any
    while (j <= r) {
        temp[k++] = arr[j++];
    }

    // Copy the sorted subarray from temp back to arr
    for (i = l; i <= r; i++) {
        arr[i] = temp[i];
    }
}

void mergeSort(int arr[], int temp[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        // Sort first and second halves
        mergeSort(arr, temp, l, m);
        mergeSort(arr, temp, m + 1, r);

        // Merge the sorted halves
        merge(arr, temp, l, m, r);
    }
}

int main(int argc, char *argv[]) {
    int n;

    if (argc != 2) {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]); 
    if (n <= 0) {
        printf("Invalid array size\n");
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
        return 1;
    }

    // Read input array from file
    for (int i = 0; i < n; i++) {
        if (fscanf(input_file, "%d, ", &arr[i]) != 1) {
            printf("Error reading from file\n");
            fclose(input_file);
            return 1;
        }
    }
    fclose(input_file);

    double total_time = 0;
    int num_trials = 3;

    for (int t = 0; t < num_trials; t++) {
        clock_t start = clock();
        mergeSort(arr, temp, 0, n - 1);
        clock_t end = clock();

        double elapsed_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        total_time += elapsed_time;
    }

    // Calculate average time
    double average_time = total_time / num_trials;
    printf("Average sorting time over %d trials: %.2f milliseconds\n", num_trials, average_time);

    // Write sorted array to output file
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
    free(arr);
    free(temp);
    return 0;
}
