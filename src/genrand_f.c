// File: src/genrand_f.c

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "common.h"

int main(int argc, char* argv[]) {
    int count = 100; // Default number of random numbers
    int min = 1;     // Default minimum value
    int max = 1000;  // Default maximum value

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            count = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-min") == 0 && i + 1 < argc) {
            min = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-max") == 0 && i + 1 < argc) {
            max = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -c COUNT    Number of random numbers to generate (default: 100)\n");
            printf("  -min MIN    Minimum value (default: 1)\n");
            printf("  -max MAX    Maximum value (default: 1000)\n");
            printf("  -h, --help  Show this help message\n");
            return 0;
        }
    }

    if (min >= max) {
        fprintf(stderr, "Error: MIN must be less than MAX\n");
        return 1;
    }

    if (count <= 0) {
        fprintf(stderr, "Error: COUNT must be greater than 0\n");
        return 1;
    }

    // Create input directory if it doesn't exist
    if (!create_directory("input")) {
        return 1;
    }

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Start timing
    clock_t start_time = clock();

    // Generate random numbers
    int* numbers = (int*)malloc(count * sizeof(int));
    if (!numbers) {
        perror("Failed to allocate memory");
        return 1;
    }

    // Generate content for hash calculation
    char* content_buffer = (char*)malloc(count * 12); // Enough space for numbers
    if (!content_buffer) {
        perror("Failed to allocate memory for content buffer");
        free(numbers);
        return 1;
    }
    content_buffer[0] = '\0';
    char temp[16];

    for (int i = 0; i < count; i++) {
        numbers[i] = min + rand() % (max - min + 1);
        sprintf(temp, "%d ", numbers[i]);
        strcat(content_buffer, temp);
    }

    // Calculate hash of contents
    uint64_t hash_value = hash_string(content_buffer);
    char filename[100];
    sprintf(filename, "input/randnum_%" PRIx64 ".txt", hash_value);

    // Write to file
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        free(numbers);
        free(content_buffer);
        return 1;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d", numbers[i]);
        if (i < count - 1) {
            fprintf(file, " ");
        }
    }

    // End timing
    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Format time
    char time_output[50];
    format_time(time_taken, time_output, sizeof(time_output));

    // Print timing information to the console but not to the file
    printf("Performance: Generated %d numbers in %s\n", count, time_output);

    fclose(file);
    printf("Generated %d random numbers between %d and %d\n", count, min, max);
    printf("Saved to file: %s\n", filename);
    printf("Performance: Generated %d numbers in %s\n", count, time_output);

    free(numbers);
    free(content_buffer);
    return 0;
}
