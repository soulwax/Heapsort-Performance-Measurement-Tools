// File: src/benchmark.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"

// Enum for algorithm type
typedef enum {
    HEAP_SORT,
    QUICK_SORT,
    BOTH_ALGORITHMS
} AlgorithmType;

// Get the time from running a sorting algorithm with time-only flag
double measure_sort_time(const char* sort_path, const char* input_file, int repeats) {
    char command[1024];
    double total_time = 0.0;

    // Create a temporary file for the time output
    char temp_output_file[] = ".temp_time_output_XXXXXX";
    int fd = mkstemp(temp_output_file);

    if (fd == -1) {
        perror("Failed to create temporary file");
        return -1.0;
    }
    close(fd);

    for (int i = 0; i < repeats; i++) {
        // Run sort with time-only mode and capture the output
        sprintf(command, "%s -f %s --time-only > %s", sort_path, input_file, temp_output_file);
        int status = system(command);

        if (status != 0) {
            printf("Error running sort command: %s\n", command);
            unlink(temp_output_file);
            return -1.0;
        }

        // Read the time output from the temporary file
        FILE* time_file = fopen(temp_output_file, "r");
        if (!time_file) {
            perror("Failed to open temporary time file");
            unlink(temp_output_file);
            return -1.0;
        }

        char time_str[100];
        if (fgets(time_str, sizeof(time_str), time_file) == NULL) {
            printf("Failed to read time output\n");
            fclose(time_file);
            unlink(temp_output_file);
            return -1.0;
        }
        fclose(time_file);

        // Parse the time value
        double time_value;
        if (strstr(time_str, "ns")) {
            sscanf(time_str, "%lf ns", &time_value);
            time_value /= 1e9;  // Convert ns to seconds
        }
        else if (strstr(time_str, "μs")) {
            sscanf(time_str, "%lf μs", &time_value);
            time_value /= 1e6;  // Convert μs to seconds
        }
        else if (strstr(time_str, "ms")) {
            sscanf(time_str, "%lf ms", &time_value);
            time_value /= 1e3;  // Convert ms to seconds
        }
        else if (strstr(time_str, "s")) {
            sscanf(time_str, "%lf s", &time_value);
        }
        else {
            printf("Unknown time format: %s\n", time_str);
            unlink(temp_output_file);
            return -1.0;
        }

        total_time += time_value;
    }

    // Clean up the temporary file
    unlink(temp_output_file);

    // Return the average time
    return total_time / repeats;
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

    for (int size = min_size; size <= max_size; size += step) {
        printf("Benchmarking array size %d... ", size);
        fflush(stdout);

        // First generate random numbers for this size (setup phase)
        char gen_cmd[512];

        // Start timing the array generation (for information only)
        clock_t gen_start_time = clock();

        sprintf(gen_cmd, "%s/genrand_f -c %d > /dev/null", bin_path, size);
        system(gen_cmd);

        // End timing the array generation
        clock_t gen_end_time = clock();
        double gen_time = (double)(gen_end_time - gen_start_time) / CLOCKS_PER_SEC;

        // Find the latest generated file
        char find_cmd[512];
        sprintf(find_cmd, "ls -t input/randnum_* | head -1 > .latest_file");
        system(find_cmd);

        FILE* latest_file = fopen(".latest_file", "r");
        if (!latest_file) {
            printf("Failed to find latest generated file\n");
            continue;
        }

        char input_file[256];
        if (fgets(input_file, sizeof(input_file), latest_file) == NULL) {
            printf("Failed to read latest file path\n");
            fclose(latest_file);
            continue;
        }
        fclose(latest_file);

        // Remove newline character
        input_file[strcspn(input_file, "\n")] = 0;

        // Create paths to the sorting binaries
        char heap_sort_path[300], quick_sort_path[300];
        sprintf(heap_sort_path, "%s/heapsort", bin_path);
        sprintf(quick_sort_path, "%s/quicksort", bin_path);

        // Run the sorting algorithm benchmark
        double heap_sort_time = -1.0;
        double quick_sort_time = -1.0;
        char heap_time_str[50] = "N/A";
        char quick_time_str[50] = "N/A";

        if (algorithm_type == HEAP_SORT || algorithm_type == BOTH_ALGORITHMS) {
            heap_sort_time = measure_sort_time(heap_sort_path, input_file, repeats);
            if (heap_sort_time < 0) {
                printf("Error measuring heapsort time\n");
                continue;
            }
            format_time(heap_sort_time, heap_time_str, sizeof(heap_time_str));
            printf("HeapSort time: %s", heap_time_str);
        }

        if (algorithm_type == QUICK_SORT || algorithm_type == BOTH_ALGORITHMS) {
            quick_sort_time = measure_sort_time(quick_sort_path, input_file, repeats);
            if (quick_sort_time < 0) {
                printf("Error measuring quicksort time\n");
                continue;
            }
            format_time(quick_sort_time, quick_time_str, sizeof(quick_time_str));
            printf("%sQuickSort time: %s", algorithm_type == BOTH_ALGORITHMS ? ", " : "", quick_time_str);
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
    }

    fclose(output_file);
    system("rm -f .latest_file");

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
    int max_size = 100000;
    int step_size = 10000;
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
