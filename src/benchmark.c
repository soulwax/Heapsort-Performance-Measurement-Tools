// File: src/benchmark.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>  // For directory operations
#include "common.h"

// Enum for algorithm type
typedef enum {
    HEAP_SORT,
    QUICK_SORT,
    BOTH_ALGORITHMS
} AlgorithmType;

// Check if a file exists and is accessible
static int file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// Find the latest generated file - use direct file search instead of relying on external commands
void find_latest_file(const char* directory, const char* prefix, char* result_path, size_t path_size) {
    DIR* dir;
    struct dirent* entry;
    time_t latest_time = 0;
    struct stat file_stat;

    // Initialize result path to empty string
    result_path[0] = '\0';

    if ((dir = opendir(directory)) == NULL) {
        fprintf(stderr, "Error opening directory: %s\n", directory);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);

            if (stat(full_path, &file_stat) == 0) {
                if (file_stat.st_mtime > latest_time) {
                    latest_time = file_stat.st_mtime;
                    strncpy(result_path, full_path, path_size);
                    result_path[path_size - 1] = '\0'; // Ensure null termination
                }
            }
        }
    }

    closedir(dir);
}

double measure_sort_time(const char* sort_path, const char* input_file, int repeats) {
    if (!file_exists(sort_path)) {
        fprintf(stderr, "Error: Sort binary not found: %s\n", sort_path);
        return -1.0;
    }

    if (!file_exists(input_file)) {
        fprintf(stderr, "Error: Input file not found: %s\n", input_file);
        return -1.0;
    }

    // Try a different approach: check file content first
    FILE* check_file = fopen(input_file, "r");
    if (!check_file) {
        fprintf(stderr, "Failed to open input file for content check: %s\n", input_file);
        return -1.0;
    }

    // Read first few bytes to validate file format
    char buffer[256];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, check_file);
    buffer[bytes_read] = '\0';
    fclose(check_file);

    // Check if file has reasonable content (should have numbers and spaces)
    int has_digits = 0;
    for (int i = 0; i < bytes_read && !has_digits; i++) {
        if (isdigit(buffer[i])) has_digits = 1;
    }

    if (!has_digits) {
        fprintf(stderr, "Warning: Input file appears to contain no digits: %s\n", input_file);
    }

    // Attempt measurement with existing file
    double total_time = 0.0;
    int successful_runs = 0;

    for (int i = 0; i < repeats; i++) {
        // Create a temporary file for capturing the output
        char temp_file[256];
        sprintf(temp_file, "/tmp/benchmark_output_%d_%d.txt", getpid(), i);

        // Use the --bench-time flag and redirect output to a file
        char command[1024];
        sprintf(command, "%s -f \"%s\" --bench-time > %s 2>/dev/null",
            sort_path, input_file, temp_file);

        // Execute the command
        int result = system(command);
        if (result != 0) {
            fprintf(stderr, "Command failed with code %d: %s\n", result, command);
            unlink(temp_file);
            continue;
        }

        // Read the output from the temporary file
        FILE* output_file = fopen(temp_file, "r");
        if (!output_file) {
            fprintf(stderr, "Failed to open output file: %s\n", temp_file);
            unlink(temp_file);
            continue;
        }

        char output_buffer[256] = { 0 };
        if (fgets(output_buffer, sizeof(output_buffer), output_file) == NULL) {
            fprintf(stderr, "No output in file: %s\n", temp_file);
            fclose(output_file);
            unlink(temp_file);
            continue;
        }
        fclose(output_file);
        unlink(temp_file);

        // Parse the time as a simple floating-point value
        double time_value;
        if (sscanf(output_buffer, "%lf", &time_value) != 1) {
            fprintf(stderr, "Failed to parse time output: %s\n", output_buffer);
            continue;
        }

        if (time_value > 0) {
            total_time += time_value;
            successful_runs++;
        }
    }

    // Return the average time if we got any successful runs
    if (successful_runs > 0) {
        return total_time / successful_runs;
    }

    // All runs failed
    return -1.0;
}

void run_algorithm_benchmark(const char* bin_path, AlgorithmType algorithm_type, int min_size, int max_size, int step, int repeats) {
    // Create benchmark results directory
    if (!create_directory("benchmark_results")) {
        return;
    }

    char output_filename[256];

    if (algorithm_type == BOTH_ALGORITHMS) {
        sprintf(output_filename, "benchmark_results/algorithm_comparison_%d_%d.csv", min_size, max_size);
    }
    else {
        const char* algo_name = (algorithm_type == HEAP_SORT) ? "heapsort" : "quicksort";
        sprintf(output_filename, "benchmark_results/%s_benchmark_%d_%d.csv", algo_name, min_size, max_size);
    }

    FILE* output_file = fopen(output_filename, "w");
    if (!output_file) {
        perror("Failed to create benchmark results file");
        return;
    }

    // Write CSV header
    if (algorithm_type == BOTH_ALGORITHMS) {
        fprintf(output_file, "Size,HeapSort Time (s),HeapSort Time (ms),HeapSort Formatted Time,QuickSort Time (s),QuickSort Time (ms),QuickSort Formatted Time,Array Generation Time (s)\n");
    }
    else {
        fprintf(output_file, "Size,Time (s),Time (ms),Formatted Time,Array Generation Time (s)\n");
    }

    const char* algo_title = (algorithm_type == HEAP_SORT) ? "HeapSort" :
        (algorithm_type == QUICK_SORT) ? "QuickSort" :
        "Algorithm Comparison";

    printf("Running %s Algorithm Benchmarks\n", algo_title);
    printf("=====================================\n");
    printf("Size range: %d to %d (step %d)\n", min_size, max_size, step);
    printf("Repetitions per size: %d\n\n", repeats);

    // Check if the required binaries exist before starting
    char heap_sort_path[300], quick_sort_path[300];
    sprintf(heap_sort_path, "%s/heapsort", bin_path);
    sprintf(quick_sort_path, "%s/quicksort", bin_path);

    if ((algorithm_type == HEAP_SORT || algorithm_type == BOTH_ALGORITHMS) && !file_exists(heap_sort_path)) {
        fprintf(stderr, "Error: HeapSort binary not found at %s\n", heap_sort_path);
        fclose(output_file);
        return;
    }

    if ((algorithm_type == QUICK_SORT || algorithm_type == BOTH_ALGORITHMS) && !file_exists(quick_sort_path)) {
        fprintf(stderr, "Error: QuickSort binary not found at %s\n", quick_sort_path);
        fclose(output_file);
        return;
    }

    // Check if genrand_f exists
    char genrand_path[300];
    sprintf(genrand_path, "%s/genrand_f", bin_path);
    if (!file_exists(genrand_path)) {
        fprintf(stderr, "Error: Random number generator binary not found at %s\n", genrand_path);
        fclose(output_file);
        return;
    }

    for (int size = min_size; size <= max_size; size += step) {
        printf("Benchmarking array size %d... ", size);
        fflush(stdout);

        // Calculate approximate memory needed (each array needs size * sizeof(int) bytes)
        size_t memory_needed = size * sizeof(int) * 4; // Original + sorted arrays in both algorithms
        if (memory_needed > 1024 * 1024 * 1024) { // More than 1GB
            printf("Warning: Size %d would require approximately %zu MB of memory\n",
                size, memory_needed / (1024 * 1024));
            printf("This exceeds reasonable limits for this benchmark. Skipping...\n");
            continue;
        }

        // Create input directory if it doesn't exist
        if (!create_directory("input")) {
            fclose(output_file);
            return;
        }

        // First generate random numbers for this size with more predictable settings
        char gen_cmd[512];
        sprintf(gen_cmd, "%s -c %d -min 1 -max 100 > /dev/null", genrand_path, size);
        if (system(gen_cmd) != 0) {
            printf("Error generating random numbers\n");
            continue;
        }

        // Find the latest generated file using our function
        char input_file[256];
        sprintf(input_file, "benchmark_input/test_%d.txt", size);

        // Check if the file exists and is readable
        if (access(input_file, R_OK) != 0) {
            printf("Error: Test file %s does not exist or cannot be read\n", input_file);
            continue;
        }

        // Skip the step of generating new random numbers for each size
        double gen_time = 0.0; // No generation time

        // Start timing the array generation (for information only)
        clock_t gen_start_time = clock();

        // Generate random numbers with more predictable settings
        sprintf(gen_cmd, "%s -c %d -min 1 -max 100 > /dev/null", genrand_path, size);        if (system(gen_cmd) != 0) {
            printf("Error generating random numbers\n");
            continue;
        }

        // End timing the array generation
        clock_t gen_end_time = clock();
        double gen_time = (double)(gen_end_time - gen_start_time) / CLOCKS_PER_SEC;

        find_latest_file("input", "randnum_", input_file, sizeof(input_file));

        if (input_file[0] == '\0') {
            printf("Failed to find latest generated file\n");
            continue;
        }

        printf("Input file: %s\n", input_file);

        // Check the file size
        struct stat file_stat;
        if (stat(input_file, &file_stat) == 0) {
            printf("Input file size: %ld bytes\n", file_stat.st_size);

            // If file is very large, try to inspect the first few lines
            if (file_stat.st_size > 1024 * 1024) { // More than 1MB
                printf("Large input file detected. Checking content format...\n");
                FILE* check_file = fopen(input_file, "r");
                if (check_file) {
                    char buffer[256];
                    if (fgets(buffer, sizeof(buffer), check_file)) {
                        printf("First line sample: %s\n", buffer);
                    }
                    fclose(check_file);
                }
            }
        }

        printf("Testing sort binaries...\n");

        // Test if the binaries work before benchmarking
        char test_cmd[512];
        sprintf(test_cmd, "./bin/heapsort -f %s --bench-time > /dev/null 2>&1", input_file);
        int result = system(test_cmd);
        if (algorithm_type == HEAP_SORT || algorithm_type == BOTH_ALGORITHMS) {
            sprintf(test_cmd, "%s -f \"%s\" --bench-time > /dev/null 2>&1", heap_sort_path, input_file);
            printf("HeapSort test: %s\n", system(test_cmd) == 0 ? "OK" : "FAILED");
        }
        if (algorithm_type == QUICK_SORT || algorithm_type == BOTH_ALGORITHMS) {
            sprintf(test_cmd, "%s -f \"%s\" --bench-time > /dev/null 2>&1", quick_sort_path, input_file);
            printf("QuickSort test: %s\n", system(test_cmd) == 0 ? "OK" : "FAILED");
        }

        // Run the sorting algorithm benchmark
        double heap_sort_time = -1.0;
        double quick_sort_time = -1.0;
        char heap_time_str[50] = "N/A";
        char quick_time_str[50] = "N/A";

        if (algorithm_type == HEAP_SORT || algorithm_type == BOTH_ALGORITHMS) {
            heap_sort_time = measure_sort_time(heap_sort_path, input_file, repeats);
            if (heap_sort_time >= 0) {
                format_time(heap_sort_time, heap_time_str, sizeof(heap_time_str));
                printf("HeapSort time: %s", heap_time_str);
            }
            else {
                // Don't break the entire benchmark if one algorithm fails
                printf("Error measuring HeapSort time");
            }
        }

        if (algorithm_type == QUICK_SORT || algorithm_type == BOTH_ALGORITHMS) {
            quick_sort_time = measure_sort_time(quick_sort_path, input_file, repeats);
            if (quick_sort_time >= 0) {
                format_time(quick_sort_time, quick_time_str, sizeof(quick_time_str));
                printf("%sQuickSort time: %s",
                    (algorithm_type == BOTH_ALGORITHMS && heap_sort_time >= 0) ? ", " : "",
                    quick_time_str);
            }
            else {
                printf("%sError measuring QuickSort time",
                    (algorithm_type == BOTH_ALGORITHMS && heap_sort_time >= 0) ? ", " : "");
            }
        }
        printf("\n");

        // Write results to CSV
        if (algorithm_type == BOTH_ALGORITHMS) {
            fprintf(output_file, "%d,%f,%f,%s,%f,%f,%s,%f\n",
                size,
                heap_sort_time, heap_sort_time * 1000, heap_time_str,
                quick_sort_time, quick_sort_time * 1000, quick_time_str,
                gen_time);
        }
        else {
            double sort_time = (algorithm_type == HEAP_SORT) ? heap_sort_time : quick_sort_time;
            const char* time_str = (algorithm_type == HEAP_SORT) ? heap_time_str : quick_time_str;

            fprintf(output_file, "%d,%f,%f,%s,%f\n",
                size, sort_time, sort_time * 1000, time_str, gen_time);
        }

        // Flush the output file to ensure partial results are saved
        fflush(output_file);
    }

    fclose(output_file);

    printf("\nBenchmark complete. Results saved to %s\n", output_filename);
    printf("Note: The benchmark focused solely on the sorting algorithm performance,\n");
    printf("      excluding file I/O operations.\n");

    if (algorithm_type == BOTH_ALGORITHMS) {
        printf("\nTo visualize the comparison results, run:\n");
        printf("python3 visualize_benchmark.py --compare %s\n", output_filename);
    }
    else {
        printf("\nTo visualize the results, run:\n");
        printf("python3 visualize_benchmark.py %s\n", output_filename);
    }
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --min SIZE           Minimum array size (default: 1000)\n");
    printf("  --max SIZE           Maximum array size (default: 100000)\n");
    printf("  --step SIZE          Step size between benchmarks (default: 10000)\n");
    printf("  --repeats N          Number of repetitions per size (default: 3)\n");
    printf("  --algorithm NAME     Algorithm to benchmark: 'heap', 'quick', or 'both' (default: 'heap')\n");
    printf("  --algorithm-compare  Compare heapsort and quicksort (shorthand for --algorithm both)\n");
    printf("  --help               Display this help message\n");
}

int main(int argc, char* argv[]) {
    int min_size = 1000;
    int max_size = 1000000;
    int step_size = 100000;
    int repeats = 3;
    AlgorithmType algorithm = HEAP_SORT;  // Default to heapsort for backward compatibility

    // Get the path to the bin directory
    char bin_path[256] = "./bin";  // Default

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
        else if (strcmp(argv[i], "--repeats") == 0 && i + 1 < argc) {
            repeats = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "heap") == 0) {
                algorithm = HEAP_SORT;
            }
            else if (strcmp(argv[i + 1], "quick") == 0) {
                algorithm = QUICK_SORT;
            }
            else if (strcmp(argv[i + 1], "both") == 0) {
                algorithm = BOTH_ALGORITHMS;
            }
            else {
                fprintf(stderr, "Error: Unknown algorithm '%s'\n", argv[i + 1]);
                print_usage(argv[0]);
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "--algorithm-compare") == 0) {
            algorithm = BOTH_ALGORITHMS;
        }
        else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    // Validate inputs
    if (min_size <= 0 || max_size <= 0 || step_size <= 0 || repeats <= 0) {
        fprintf(stderr, "Error: All size and repeat parameters must be positive\n");
        return 1;
    }

    if (min_size > max_size) {
        fprintf(stderr, "Error: Minimum size must be less than or equal to maximum size\n");
        return 1;
    }

    // Run the benchmark
    run_algorithm_benchmark(bin_path, algorithm, min_size, max_size, step_size, repeats);

    return 0;
}
