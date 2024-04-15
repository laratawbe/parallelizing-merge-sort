#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int n = 100000000; // Total number of integers
    const char *filename = "array1.txt";
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < n; i++) {
        int num = rand() % (n + 1); // Generate a random number between 0 and n
        fprintf(file, "%d", num); // Write the number to the file

        if (i < n - 1) {
            fprintf(file, ", "); // Add a comma and space after each number except the last one
        }
    }

    fclose(file);
    printf("File '%s' created successfully.\n", filename);

    return 0;
}
