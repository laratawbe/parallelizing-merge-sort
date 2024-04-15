#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

#define THREADS_PER_BLOCK 256

// CUDA kernel to merge two sorted halves of an array
__device__ void merge(int *arr, int *temp, int l, int m, int r)
{
    int n1 = m - l + 1;
    int n2 = r - m;

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2)
    {
        if (arr[l + i] <= arr[m + 1 + j])
            temp[k++] = arr[l + i++];
        else
            temp[k++] = arr[m + 1 + j++];
    }

    while (i < n1)
        temp[k++] = arr[l + i++];

    while (j < n2)
        temp[k++] = arr[m + 1 + j++];

    for (i = l; i <= r; i++)
        arr[i] = temp[i];
}

// CUDA kernel to perform merge sort on the device
__global__ void mergeSort(int *arr, int *temp, int size)
{
    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int start = tid * size;

    for (int curr_size = 1; curr_size < size; curr_size *= 2)
    {
        for (int left_start = start; left_start < size - 1; left_start += 2 * curr_size)
        {
            int mid = left_start + curr_size - 1;
            int right_end = min(left_start + 2 * curr_size - 1, size - 1);

            if (mid < right_end)
                merge(arr, temp, left_start, mid, right_end);
        }
    }
}

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

    int *arr, *d_arr, *temp;
    arr = (int *)malloc(n * sizeof(int));
    temp = (int *)malloc(n * sizeof(int));

    // Generate random input data (for demonstration)
    if (arr == NULL || temp == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

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
        if (fscanf(input_file, "%d, ", &arr[i]) != 1)
        {
            printf("Error reading from file\n");
            fclose(input_file);
            free(arr);
            free(temp);
            return 1;
        }
    }

    cudaMalloc(&d_arr, n * sizeof(int));
    cudaMalloc(&temp, n * sizeof(int));

    cudaMemcpy(d_arr, arr, n * sizeof(int), cudaMemcpyHostToDevice);

    // Determine grid and block dimensions based on the input size
    int num_threads = min(n, THREADS_PER_BLOCK);
    int num_blocks = (n + num_threads - 1) / num_threads;
    printf("%d", num_blocks);
    clock_t total_time = 0;
    int num_trials = 100;

    for (int t = 0; t < num_trials; t++)
    {
        cudaMemcpy(d_arr, arr, n * sizeof(int), cudaMemcpyHostToDevice);

        clock_t start = clock();

        mergeSort<<<num_blocks, num_threads>>>(d_arr, temp, n);

        cudaDeviceSynchronize();

        clock_t end = clock();

        double elapsed_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        total_time += elapsed_time;

        printf("Trial %d: Sorting time - %.2f milliseconds\n", t + 1, elapsed_time);
    }

    double average_time = (double)total_time / num_trials;
    printf("\nAverage sorting time over %d trials: %.2f milliseconds\n", num_trials, average_time);

    cudaMemcpy(arr, d_arr, n * sizeof(int), cudaMemcpyDeviceToHost);

    FILE *output_file = fopen("sorted.txt", "w");
    if (output_file == NULL)
    {
        printf("Error opening file: sorted.txt\n");
        cudaFree(d_arr);
        cudaFree(temp);
        free(arr);
        free(temp);
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        fprintf(output_file, "%d, ", arr[i]);
    }

    fclose(output_file);
    cudaFree(d_arr);
    cudaFree(temp);
    free(arr);
    free(temp);

    return 0;
}
