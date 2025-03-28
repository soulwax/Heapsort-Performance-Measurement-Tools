// File: src/generate_test_files.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "common.h"

int main(int argc, char* argv[]) {
    // Default range for array sizes
    int min_size = 1000;
    int max_size = 50000;
    int step_size = 5000;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--min") == 0 && i + 1 < argc) {
            min_size = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--max") == 0 && i + 1 < argc) {
            max_size = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--step") == 0 && i + 1 < argc) {
            step_size = atoi(argv[i + 1]);
            i++;
        }
    }

    printf("Generating test files...\n");
    printf("Size range: %d to %d (step %d)\n", min_size, max_size, step_size);

    // Create input directory if it doesn't exist
    if (!create_directory("benchmark_input")) {
        return 1;
    }

    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Generate a test file for each size
    for (int size = min_size; size <= max_size; size += step_size) {
        printf("Generating file for size %d... ", size);

        // Generate random numbers
        int* numbers = (int*)malloc(size * sizeof(int));
        if (!numbers) {
            perror("Failed to allocate memory");
            continue;
        }

        // Use a simple pattern - alternating values in a small range
        // This avoids potential issues with very large or pathological arrays
        for (int i = 0; i < size; i++) {
            numbers[i] = rand() % 1000;  // Small range of values
        }

        // Create a filename based on the size
        char filename[256];
        sprintf(filename, "benchmark_input/test_%d.txt", size);

        // Write to file
        FILE* file = fopen(filename, "w");
        if (!file) {
            perror("Failed to open file for writing");
            free(numbers);
            continue;
        }

        // Write numbers with spaces between them
        for (int i = 0; i < size; i++) {
            fprintf(file, "%d", numbers[i]);
            if (i < size - 1) {
                fprintf(file, " ");
            }

            // Add newlines every 20 numbers for readability
            if ((i + 1) % 20 == 0) {
                fprintf(file, "\n");
            }
        }

        fclose(file);

        // Open the file and validate it directly
        FILE* check_file = fopen(filename, "r");
        if (!check_file) {
            printf("FAILED (Could not open file)\n");
            free(numbers);
            continue;
        }

        // Count the actual number of integers in the file
        int test_num, test_count = 0;
        while (fscanf(check_file, "%d", &test_num) == 1) {
            test_count++;
        }
        fclose(check_file);

        if (test_count == size) {
            printf("OK (%d integers verified)\n", test_count);
        }
        else {
            printf("FAILED (Expected %d integers, found %d)\n", size, test_count);
        }

        free(numbers);
    }

    printf("\nTest file generation complete.\n");
    return 0;
}
