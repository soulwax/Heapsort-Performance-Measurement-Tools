// File: src/quicksort_f.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>  // For errno and strerror
#include "common.h"

// Define cache line size for optimizations
#define CACHE_LINE_SIZE 64

// Prefetch hint macros
#if defined(__GNUC__) || defined(__clang__)
#define PREFETCH(addr) __builtin_prefetch(addr)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define PREFETCH(addr)
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

// Threshold for switching to insertion sort
#define INSERTION_SORT_THRESHOLD 16

// Optimized insertion sort for small arrays
static void insertion_sort(int a[], int start, int end) {
    for (int i = start + 1; i <= end; i++) {
        int key = a[i];
        int j = i - 1;

        while (j >= start && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

// Optimized swap function
static inline void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Median of three pivot selection
static int median_of_three(int a[], int low, int high) {
    int mid = low + (high - low) / 2;

    // Sort the three elements
    if (a[mid] < a[low])
        swap(&a[low], &a[mid]);
    if (a[high] < a[low])
        swap(&a[low], &a[high]);
    if (a[high] < a[mid])
        swap(&a[mid], &a[high]);

    // Return the middle element as pivot
    return a[mid];
}

// Partition function for quicksort with median-of-three pivot
static int partition(int a[], int low, int high) {
    // Choose pivot using median of three
    int pivot = median_of_three(a, low, high);

    // Move pivot to the high position
    swap(&a[high], &a[high - 1]);

    // Use the value, not the position
    pivot = a[high];

    // Prefetch next cache line
    if (high - low > CACHE_LINE_SIZE / sizeof(int)) {
        PREFETCH(&a[low + CACHE_LINE_SIZE / sizeof(int)]);
    }

    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (LIKELY(a[j] <= pivot)) {
            i++;
            swap(&a[i], &a[j]);
        }
    }
    swap(&a[i + 1], &a[high]);
    return (i + 1);
}

// Tail-recursive quicksort to reduce stack space
static void quicksort_internal(int a[], int low, int high) {
    while (low < high) {
        // For small arrays, use insertion sort
        if (high - low < INSERTION_SORT_THRESHOLD) {
            insertion_sort(a, low, high);
            break;
        }

        // Partition the array
        int pi = partition(a, low, high);

        // Optimize tail recursion
        // Recursively sort the smaller partition
        if (pi - low < high - pi) {
            quicksort_internal(a, low, pi - 1);
            low = pi + 1;  // Tail recursion for the larger partition
        }
        else {
            quicksort_internal(a, pi + 1, high);
            high = pi - 1;  // Tail recursion for the larger partition
        }
    }
}

// Public quicksort function
void quickSort(int a[], int n) {
    if (n <= 1) return;
    quicksort_internal(a, 0, n - 1);
}


void printUsage(char* programName) {
    printf("Usage:\n");
    printf("  %s <num1> <num2> <num3> ...          # Sort numbers from command line\n", programName);
    printf("  %s -f <input_file>                   # Sort numbers from input file\n", programName);
    printf("  %s -f <input_file> -o <output_file>  # Sort numbers from input file and write to output file\n", programName);
    printf("  %s -f <input_file> --time-only       # Only output the sorting time (for benchmarking)\n", programName);
    printf("  %s -f <input_file> --bench-time      # Output raw time value for benchmark tool\n", programName);
}

int main(int argc, char* argv[]) {
    int* a = NULL;
    int n = 0;
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    char* inputFilename = NULL;
    int timeOnly = 0;
    int benchTimeMode = 0;
    int usingFiles = 0;
    FILE* debug_log = fopen("sort_debug.log", "a");
    if (debug_log) {
        fprintf(debug_log, "--- %s Debug Log ---\n", argv[0]);
        fprintf(debug_log, "Command line: ");
        for (int i = 0; i < argc; i++) {
            fprintf(debug_log, "%s ", argv[i]);
        }
        fprintf(debug_log, "\n");
    }
    // Parse command line arguments in a single pass
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--bench-time") == 0) {
            benchTimeMode = 1;
            timeOnly = 0;  // bench-time takes precedence
        }
        else if (strcmp(argv[i], "--time-only") == 0) {
            if (!benchTimeMode) {
                timeOnly = 1;
            }
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            inputFilename = argv[i + 1];
            i++; // Skip the filename in the next iteration

            usingFiles = 1;

            // Check for output file option that follows
            if (i + 2 < argc && strcmp(argv[i + 1], "-o") == 0) {
                outputFile = fopen(argv[i + 2], "w");
                if (outputFile == NULL) {
                    fprintf(stderr, "Error: Could not open output file '%s'\n", argv[i + 2]);
                    return 1;
                }
                i += 2; // Skip both -o and the output filename
            }
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            // Handle -o when it's not immediately after -f
            outputFile = fopen(argv[i + 1], "w");
            if (outputFile == NULL) {
                fprintf(stderr, "Error: Could not open output file '%s'\n", argv[i + 1]);
                return 1;
            }
            i++; // Skip the filename in the next iteration
        }
    }

    // Open the input file after parsing all arguments
    if (usingFiles) {
        if (inputFilename == NULL) {
            fprintf(stderr, "Error: Input filename is required with -f flag\n");
            return 1;
        }

        inputFile = fopen(inputFilename, "r");
        if (inputFile == NULL) {
            if (debug_log) {
                fprintf(debug_log, "Error: Could not open input file '%s' (errno: %d - %s)\n",
                    inputFilename, errno, strerror(errno));
                fclose(debug_log);
            }
            fprintf(stderr, "Error: Could not open input file '%s' (errno: %d - %s)\n",
                inputFilename, errno, strerror(errno));
            return 1;
        }
    }

    // Validate arguments - must have either numbers or input file
    if (argc < 2 || (!usingFiles && argc == 2 && (timeOnly || benchTimeMode))) {
        printUsage(argv[0]);
        return 1;
    }

    if (usingFiles) {
        // Read integers from file
        a = readIntegers(inputFile, &n);
        if (a == NULL || n == 0) {
            if (debug_log) {
                fprintf(debug_log, "Error: Could not read integers from file: %s (count: %d)\n",
                    inputFilename, n);
                fclose(debug_log);
            }
            printf("Error: No valid integers found in the input file\n");
            return 1;
        }
        fclose(inputFile);

        if (a == NULL || n == 0) {
            printf("Error: No valid integers found in the input file\n");
            return 1;
        }
    }
    else {
        // Parse command line numbers
        n = argc - 1;
        // Align the array for better memory access
        a = (int*)aligned_alloc(CACHE_LINE_SIZE, ((n + 7) & ~7) * sizeof(int));

        if (a == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }

        for (int i = 0; i < n; i++) {
            a[i] = atoi(argv[i + 1]);
        }
    }

    // Create a copy of the original array for displaying
    int* original = (int*)aligned_alloc(CACHE_LINE_SIZE, ((n + 7) & ~7) * sizeof(int));
    if (original == NULL) {
        printf("Memory allocation failed\n");
        free(a);
        return 1;
    }
    memcpy(original, a, n * sizeof(int));

    // Time measurement - start
    clock_t start_time = clock();

    // Sort the array using QuickSort
    quickSort(a, n);

    // Time measurement - end
    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Benchmark time mode has highest priority and uses a very specific format
    if (benchTimeMode) {
        // Output ONLY the raw time value in seconds with consistent format
        printf("%.9f\n", time_taken); // Use fixed precision for consistency
        free(original);
        free(a);
        return 0;
    }

    // Format time output for normal display
    char time_output[50];
    format_time(time_taken, time_output, sizeof(time_output));

    // Time-only mode has second priority
    if (timeOnly) {
        printf("%s\n", time_output);
        free(original);
        free(a);
        return 0;
    }

    // If using input file, create/use output file in output directory
    if (usingFiles && outputFile == NULL) {
        // Create output directory if it doesn't exist
        if (!create_directory("output")) {
            free(a);
            free(original);
            return 1;
        }

        // Extract the filename from the input path
        const char* baseFilename = get_filename(inputFilename);

        // Create output filename
        char outputPath[1024];
        snprintf(outputPath, sizeof(outputPath), "output/%s", baseFilename);

        outputFile = fopen(outputPath, "w");
        if (outputFile == NULL) {
            printf("Error: Could not create output file '%s'\n", outputPath);
            free(a);
            free(original);
            return 1;
        }

        printf("Writing sorted output to: %s\n", outputPath);
    }

    if (outputFile != NULL) {
        // Write both original and sorted arrays to the output file
        fprintf(outputFile, "Original array: ");
        writeIntegers(outputFile, original, n);

        fprintf(outputFile, "Sorted array: ");
        writeIntegers(outputFile, a, n);

        // Write performance information - only the sorting algorithm time
        fprintf(outputFile, "Sorting algorithm performance: Sorted %d items in %s\n", n, time_output);
        fprintf(outputFile, "Algorithm: Optimized QuickSort with median-of-three pivot\n");

        fclose(outputFile);
    }
    else {
        // Print to console if no output file specified
        printf("Original array: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", original[i]);
            if ((i + 1) % 20 == 0) printf("\n");
        }
        printf("\n");

        printf("Sorted array: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", a[i]);
            if ((i + 1) % 20 == 0) printf("\n");
        }
        printf("\n");

        // Print performance information
        printf("Sorting algorithm performance: Sorted %d items in %s\n", n, time_output);
        printf("Algorithm: Optimized QuickSort with median-of-three pivot\n");
    }

    free(original);
    free(a);
    return 0;
}
