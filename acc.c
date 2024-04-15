#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THREADS_PER_BLOCK 256

// Function prototypes
void mergeSort(int arr[], int l, int r);
void merge(int arr[], int l, int m, int r);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0)
    {
        printf("Invalid array size\n");
        return 1;
    }

    int *arr = (int *)malloc(n * sizeof(int));
    int *temp = (int *)malloc(n * sizeof(int));

    FILE *input_file = fopen("array1.txt", "r");
    if (input_file == NULL)
    {
        printf("Error opening file: array1.txt\n");
        free(arr);
        free(temp);
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        if (fscanf(input_file, "%d,", &arr[i]) != 1)
        {
            printf("Error reading from file\n");
            fclose(input_file);
            free(arr);
            free(temp);
            return 1;
        }
    }

    fclose(input_file);

    clock_t total_time = 0;
    int num_trials = 100;

    for (int t = 0; t < num_trials; t++)
    {
        // Copy original array to temp array for each trial
        for (int i = 0; i < n; i++)
        {
            temp[i] = arr[i];
        }

        clock_t start = clock();

        // Perform Merge Sort with OpenACC
#pragma acc data copy(temp[0 : n])
        {
            mergeSort(temp, 0, n - 1);
        }

        clock_t end = clock();
        double elapsed_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        total_time += elapsed_time;

        // printf("Trial %d: Sorting time - %.2f milliseconds\n", t + 1, elapsed_time);
    }

    // Calculate average sorting time over all trials
    double average_time = (double)total_time / num_trials;
    printf("\nAverage sorting time over %d trials: %.2f milliseconds\n", num_trials, average_time);

    // Write sorted array to file (using the last sorted temp array)
    FILE *output_file = fopen("sorted.txt", "w");
    if (output_file == NULL)
    {
        printf("Error opening file: sorted.txt\n");
        free(arr);
        free(temp);
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        fprintf(output_file, "%d, ", temp[i]);
    }

    fclose(output_file);
    free(arr);
    free(temp);

    return 0;
}

// Function to merge two halves of an array
void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    int *L = malloc(n1 * sizeof(int)); // Dynamically allocate memory for L
    int *R = malloc(n2 * sizeof(int)); // Dynamically allocate memory for R

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

// Parallelized Merge Sort using OpenACC
void mergeSort(int arr[], int l, int r)
{
    if (l < r)
    {
        int m = l + (r - l) / 2;

        // Perform Merge Sort on first half
#pragma acc parallel present(arr[l : m + 1])
        mergeSort(arr, l, m);

        // Perform Merge Sort on second half
#pragma acc parallel present(arr[m + 1 : r + 1])
        mergeSort(arr, m + 1, r);

        // Merge the sorted halves
        merge(arr, l, m, r);
    }
}
