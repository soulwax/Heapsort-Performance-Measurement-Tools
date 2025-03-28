// File: src/heapsort_f.c

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
static void insertion_sort(int a[], int n) {
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;

        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

// Optimized swap with branch prediction hints
static inline void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Cache-optimized non-recursive heapify function
static void heapify(int a[], int n, int i) {
    int current = a[i];  // Cache the current value to avoid repeated memory access

    while (1) {
        int child_idx = (i << 1) + 1;  // Left child
        if (child_idx >= n) break;

        // Prefetch the next level of children
        if (child_idx + 2 < n) {
            PREFETCH(&a[(child_idx << 1) + 1]);
            PREFETCH(&a[(child_idx << 1) + 2]);
        }

        // Determine the larger child
        if (child_idx + 1 < n && a[child_idx] < a[child_idx + 1]) {
            child_idx++;
        }

        // If parent is larger than the largest child, we're done
        if (current >= a[child_idx]) break;

        // Move child up
        a[i] = a[child_idx];
        i = child_idx;
    }

    // Place current value in its final position
    a[i] = current;
}

// Build max heap with cache-friendly traversal
static void buildMaxHeap(int a[], int n) {
    // Start from last non-leaf node and heapify all nodes in reverse order
    for (int i = (n >> 1) - 1; i >= 0; i--) {
        heapify(a, n, i);
    }
}

// Hybrid heap sort with insertion sort for small arrays
void heapSort(int a[], int n) {
    // For very small arrays, use insertion sort directly
    if (UNLIKELY(n <= INSERTION_SORT_THRESHOLD)) {
        insertion_sort(a, n);
        return;
    }

    // Build a max heap
    buildMaxHeap(a, n);

    // Extract elements from the heap one by one
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        swap(&a[0], &a[i]);

        // If the remaining heap is small, use insertion sort
        if (UNLIKELY(i <= INSERTION_SORT_THRESHOLD)) {
            insertion_sort(a, i);
            break;
        }

        // Call heapify on the reduced heap
        heapify(a, i, 0);
    }
}

// Optimized version that processes elements in cache-friendly blocks
void blockHeapSort(int a[], int n) {
    if (n <= INSERTION_SORT_THRESHOLD) {
        insertion_sort(a, n);
        return;
    }

    // Build heap
    buildMaxHeap(a, n);

    // Calculate block size based on cache line
    const int block_size = CACHE_LINE_SIZE / sizeof(int);

    // Process blocks of elements at a time
    for (int end = n - 1; end > 0;) {
        int block_end = (end > block_size) ? end - block_size : 0;

        // Process current block
        for (int i = end; i > block_end; i--) {
            swap(&a[0], &a[i]);
            heapify(a, i, 0);
        }

        end = block_end;
    }
}



void printUsage(char* programName) {
    printf("Usage:\n");
    printf("  %s <num1> <num2> <num3> ...          # Sort numbers from command line\n", programName);
    printf("  %s -f <input_file>                   # Sort numbers from input file\n", programName);
    printf("  %s -f <input_file> -o <output_file>  # Sort numbers from input file and write to output file\n", programName);
    printf("  %s -f <input_file> --time-only       # Only output the sorting time (for benchmarking)\n", programName);
    printf("  %s -f <input_file> --bench-time      # Output raw time value for benchmark tool\n", programName);
    printf("  %s -f <input_file> --block-sort      # Use cache-optimized block sorting algorithm\n", programName);
}

int main(int argc, char* argv[]) {
    int* a = NULL;
    int n = 0;
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    char* inputFilename = NULL;
    int timeOnly = 0;
    int benchTimeMode = 0;
    int useBlockSort = 0;
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
        else if (strcmp(argv[i], "--block-sort") == 0) {
            useBlockSort = 1;
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
    if (argc < 2 || (!usingFiles && argc == 2 && (timeOnly || benchTimeMode || useBlockSort))) {
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

    // Choose sorting algorithm based on flag
    if (useBlockSort) {
        blockHeapSort(a, n);
    }
    else {
        heapSort(a, n);
    }

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
        if (useBlockSort) {
            fprintf(outputFile, "Algorithm: Cache-optimized Block Heap Sort\n");
        }
        else {
            fprintf(outputFile, "Algorithm: Optimized Hybrid Heap Sort\n");
        }

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
        if (useBlockSort) {
            printf("Algorithm: Cache-optimized Block Heap Sort\n");
        }
        else {
            printf("Algorithm: Optimized Hybrid Heap Sort\n");
        }
    }

    free(original);
    free(a);
    return 0;
}
