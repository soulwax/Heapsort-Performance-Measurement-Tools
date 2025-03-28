// File: src/quicksort_f.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
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

// Count the number of integers in a file - optimized version with single pass
int countIntegers(FILE* file) {
    int count = 0;
    char buffer[4096];  // Larger buffer for better I/O performance

    // Reset file position to the beginning
    rewind(file);

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        int in_number = 0;

        for (char* p = buffer; *p; p++) {
            if (isdigit(*p) || (*p == '-' || *p == '+')) {
                if (!in_number) {
                    count++;
                    in_number = 1;
                }
            }
            else if (!isspace(*p)) {
                in_number = 0;
            }
            else {
                in_number = 0;
            }
        }
    }

    // Reset file position to the beginning
    rewind(file);
    return count;
}

// Read integers from a file into an array - optimized for fewer passes
int* readIntegers(FILE* file, int* count) {
    *count = countIntegers(file);

    if (*count == 0)
        return NULL;

    // Align to cache line boundary for better memory access
    int* array = (int*)aligned_alloc(CACHE_LINE_SIZE, ((*count + 7) & ~7) * sizeof(int));
    if (array == NULL)
        return NULL;

    int index = 0;
    char buffer[4096];  // Larger buffer for better I/O performance

    rewind(file);
    while (fgets(buffer, sizeof(buffer), file) != NULL && index < *count) {
        char* token = strtok(buffer, " \t\n,;");
        while (token != NULL && index < *count) {
            // Use strtol for more robust conversion
            char* endptr;
            long val = strtol(token, &endptr, 10);

            // Only add valid integers
            if (endptr != token) {
                array[index++] = (int)val;
            }

            token = strtok(NULL, " \t\n,;");
        }
    }

    *count = index;  // Update with actual count of values read
    return array;
}

// Write sorted integers to a file
void writeIntegers(FILE* file, int array[], int count) {
    const int ITEMS_PER_LINE = 20;
    for (int i = 0; i < count; i++) {
        fprintf(file, "%d", array[i]);

        // Add space if not last item
        if (i < count - 1) {
            fprintf(file, " ");

            // Add newline for better readability
            if ((i + 1) % ITEMS_PER_LINE == 0) {
                fprintf(file, "\n");
            }
        }
    }
    fprintf(file, "\n");
}

void printUsage(char* programName) {
    printf("Usage:\n");
    printf("  %s <num1> <num2> <num3> ...          # Sort numbers from command line\n", programName);
    printf("  %s -f <input_file>                   # Sort numbers from input file\n", programName);
    printf("  %s -f <input_file> -o <output_file>  # Sort numbers from input file and write to output file\n", programName);
    printf("  %s -f <input_file> --time-only       # Only output the sorting time (for benchmarking)\n", programName);
}

int main(int argc, char* argv[]) {
    int* a = NULL;
    int n = 0;
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    int usingFiles = 0;
    char* inputFilename = NULL;
    int timeOnly = 0;     // Flag for benchmark mode

    // Parse command line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    // Check for flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--time-only") == 0) {
            timeOnly = 1;
        }
    }

    // Check if using file input
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            inputFilename = argv[i + 1];
            inputFile = fopen(inputFilename, "r");
            if (inputFile == NULL) {
                printf("Error: Could not open input file '%s'\n", inputFilename);
                return 1;
            }
            usingFiles = 1;

            // Check for output file
            if (i + 2 < argc && strcmp(argv[i + 2], "-o") == 0 && i + 3 < argc) {
                outputFile = fopen(argv[i + 3], "w");
                if (outputFile == NULL) {
                    printf("Error: Could not open output file '%s'\n", argv[i + 3]);
                    fclose(inputFile);
                    return 1;
                }
            }
            break;
        }
    }

    if (usingFiles) {
        // Read integers from file
        a = readIntegers(inputFile, &n);
        fclose(inputFile);

        if (a == NULL || n == 0) {
            printf("Error: No valid integers found in the input file\n");
            return 1;
        }
    }
    else {
        // Use command line arguments as before
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

    // Format time output
    char time_output[50];
    format_time(time_taken, time_output, sizeof(time_output));

    // In time-only mode, just print the time and exit
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
