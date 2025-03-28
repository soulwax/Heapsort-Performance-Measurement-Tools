// File: src/benchmark.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

// Get the time from running heapsort with time-only flag
double measure_heapsort_time(const char* heapsort_path, const char* input_file, int repeats) {
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

    // Diagnostic: Check input file details
    FILE* input_check = fopen(input_file, "r");
    if (!input_check) {
        fprintf(stderr, "ERROR: Cannot open input file '%s': %s\n", input_file, strerror(errno));
        unlink(temp_output_file);
        return -1.0;
    }

    // Check file size and first few characters
    fseek(input_check, 0, SEEK_END);
    long file_size = ftell(input_check);
    rewind(input_check);

    // Diagnostic: Read first few characters
    char first_chars[100] = { 0 };
    size_t chars_read = fread(first_chars, 1, sizeof(first_chars) - 1, input_check);
    fclose(input_check);

    fprintf(stderr, "Input file details:\n");
    fprintf(stderr, "  Path: %s\n", input_file);
    fprintf(stderr, "  Size: %ld bytes\n", file_size);
    fprintf(stderr, "  First characters: '%s'\n", first_chars);

    for (int i = 0; i < repeats; i++) {
        // Diagnostic: Build and print the full command
        sprintf(command, "%s -f \"%s\" --time-only > %s 2>&1", heapsort_path, input_file, temp_output_file);
        fprintf(stderr, "Executing command: %s\n", command);

        // Run the command and capture both stdout and stderr
        int status = system(command);

        if (status != 0) {
            fprintf(stderr, "ERROR: Heapsort command failed with status %d\n", status);

            // Read and print the error output
            FILE* error_log = fopen(temp_output_file, "r");
            if (error_log) {
                char error_buffer[1024];
                while (fgets(error_buffer, sizeof(error_buffer), error_log)) {
                    fprintf(stderr, "Command output: %s", error_buffer);
                }
                fclose(error_log);
            }

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
            fprintf(stderr, "ERROR: Failed to read time output from %s\n", temp_output_file);
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
            fprintf(stderr, "ERROR: Unknown time format: %s\n", time_str);
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

void run_heapsort_benchmark(const char* bin_path, int min_size, int max_size, int step, int repeats) {
    // Create benchmark results directory
    if (!create_directory("benchmark_results")) {
        return;
    }

    char output_filename[256];
    sprintf(output_filename, "benchmark_results/heapsort_benchmark_%d_%d.csv", min_size, max_size);

    FILE* output_file = fopen(output_filename, "w");
    if (!output_file) {
        perror("Failed to create benchmark results file");
        return;
    }

    // Write CSV header
    fprintf(output_file, "Size,Time (s),Time (ms),Formatted Time,Array Generation Time (s)\n");

    printf("Running HeapSort Algorithm Benchmarks\n");
    printf("=====================================\n");
    printf("Size range: %d to %d (step %d)\n", min_size, max_size, step);
    printf("Repetitions per size: %d\n\n", repeats);

    // Ensure heapsort executable exists and is executable
    char heapsort_full_path[512];
    snprintf(heapsort_full_path, sizeof(heapsort_full_path), "%s/heapsort", bin_path);

    if (access(heapsort_full_path, X_OK) != 0) {
        fprintf(stderr, "Error: Heapsort executable not found or not executable at %s\n", heapsort_full_path);
        fclose(output_file);
        return;
    }

    for (int size = min_size; size <= max_size; size += step) {
        printf("Benchmarking array size %d... ", size);
        fflush(stdout);

        // First generate random numbers for this size (setup phase)
        char gen_cmd[512];

        // Start timing the array generation (for information only)
        clock_t gen_start_time = clock();

        sprintf(gen_cmd, "%s/genrand_f -c %d > /dev/null", bin_path, size);
        int gen_status = system(gen_cmd);

        if (gen_status != 0) {
            printf("Failed to generate random numbers\n");
            continue;
        }

        // End timing the array generation
        clock_t gen_end_time = clock();
        double gen_time = (double)(gen_end_time - gen_start_time) / CLOCKS_PER_SEC;

        // Find the latest generated file - use a more robust method
        char find_cmd[512];
        sprintf(find_cmd, "ls -t input/randnum_* | head -1 > .latest_file 2>/dev/null");
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

        // Run the sorting algorithm benchmark with retry mechanism
        double sort_time = -1.0;
        int max_attempts = 3;
        for (int attempt = 0; attempt < max_attempts && sort_time < 0; attempt++) {
            sort_time = measure_heapsort_time(bin_path, input_file, repeats);

            if (sort_time < 0) {
                printf("Attempt %d failed. ", attempt + 1);

                // Try regenerating the input file if it fails
                sprintf(gen_cmd, "%s/genrand_f -c %d > /dev/null", bin_path, size);
                system(gen_cmd);
            }
        }

        if (sort_time < 0) {
            printf("Failed to measure sort time after %d attempts\n", max_attempts);
            continue;
        }

        // Format time for display
        char time_str[50];
        format_time(sort_time, time_str, sizeof(time_str));

        // Write results to CSV
        fprintf(output_file, "%d,%f,%f,%s,%f\n",
            size, sort_time, sort_time * 1000, time_str, gen_time);

        printf("Sort time: %s\n", time_str);

        // Flush file to ensure results are saved periodically
        fflush(output_file);
    }

    fclose(output_file);
    system("rm -f .latest_file");

    printf("\nBenchmark complete. Results saved to %s\n", output_filename);
    printf("Note: The benchmark focused solely on the sorting algorithm performance,\n");
    printf("      excluding file I/O operations.\n");
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --min SIZE       Minimum array size (default: 1000)\n");
    printf("  --max SIZE       Maximum array size (default: 100000)\n");
    printf("  --step SIZE      Step size between benchmarks (default: 10000)\n");
    printf("  --repeats N      Number of repetitions per size (default: 3)\n");
    printf("  --help           Display this help message\n");
}

int main(int argc, char* argv[]) {
    int min_size = 1000;
    int max_size = 100000;
    int step_size = 1000;
    int repeats = 3;

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

    // Get the path to the bin directory
    char bin_path[256] = "./bin";  // Default

    // Run the benchmark
    run_heapsort_benchmark(bin_path, min_size, max_size, step_size, repeats);

    return 0;
}
