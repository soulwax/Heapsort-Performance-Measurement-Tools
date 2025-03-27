// File: src/benchmark.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

// Format time into appropriate units (ns, μs, ms, s)
void format_time(double time_seconds, char* buffer, size_t buffer_size) {
    if (time_seconds < 0.000001) {
        // Nanoseconds (less than 1 microsecond)
        sprintf(buffer, "%.2f ns", time_seconds * 1e9);
    }
    else if (time_seconds < 0.001) {
        // Microseconds (less than 1 millisecond)
        sprintf(buffer, "%.2f μs", time_seconds * 1e6);
    }
    else if (time_seconds < 1.0) {
        // Milliseconds (less than 1 second)
        sprintf(buffer, "%.2f ms", time_seconds * 1e3);
    }
    else {
        // Seconds
        sprintf(buffer, "%.2f s", time_seconds);
    }
}

// Create directory if it doesn't exist
int create_directory(const char* path) {
    struct stat st = { 0 };

    if (stat(path, &st) == -1) {
#ifdef _WIN32
        if (mkdir(path) != 0) {
#else
        if (mkdir(path, 0755) != 0) {
#endif
            perror("Failed to create directory");
            return 0;
        }
        printf("Created directory: %s\n", path);
    }

    return 1;
}

void run_heapsort_benchmark(const char* heapsort_path, int min_size, int max_size, int step, int repeats) {
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
    fprintf(output_file, "Size,Time (s),Time (ms),Formatted Time\n");

    printf("Running HeapSort benchmarks from size %d to %d (step %d, %d repeats)...\n",
        min_size, max_size, step, repeats);

    for (int size = min_size; size <= max_size; size += step) {
        printf("Benchmarking size %d... ", size);

        // First generate random numbers for this size
        char gen_cmd[512];
        sprintf(gen_cmd, "%s/gen_randf -c %d > /dev/null", heapsort_path, size);
        system(gen_cmd);

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

        // Run the benchmark multiple times and take average
        double total_time = 0.0;
        char heapsort_cmd[512];

        for (int rep = 0; rep < repeats; rep++) {
            // Create a temporary file for timing output
            sprintf(heapsort_cmd, "%s/heapsort -f %s -o .temp_sort_output", heapsort_path, input_file);

            clock_t start_time = clock();
            system(heapsort_cmd);
            clock_t end_time = clock();

            total_time += (double)(end_time - start_time) / CLOCKS_PER_SEC;
        }

        // Calculate average time
        double avg_time = total_time / repeats;

        // Format time for display
        char time_str[50];
        format_time(avg_time, time_str, sizeof(time_str));

        // Write results to CSV
        fprintf(output_file, "%d,%f,%f,%s\n",
            size, avg_time, avg_time * 1000, time_str);

        printf("Average time: %s\n", time_str);

        // Clean up temporary files
        system("rm -f .temp_sort_output");
    }

    fclose(output_file);
    system("rm -f .latest_file");

    printf("Benchmark complete. Results saved to %s\n", output_filename);
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
    int step_size = 10000;
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
