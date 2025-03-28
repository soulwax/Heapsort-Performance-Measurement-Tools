// File: src/common.c

#include "common.h"

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

// Function to extract filename from path
const char* get_filename(const char* path) {
    const char* filename = strrchr(path, '/');

    if (filename != NULL) {
        // Move past the '/'
        return filename + 1;
    }

    return path; // No '/' found, return the original path
}

// Simple hash function (djb2)
uint64_t hash_string(const char* str) {
    uint64_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

// Count the number of integers in a file
int countIntegers(FILE * file) {
    int count = 0;
    int num;

    // Reset file position to beginning
    rewind(file);

    // Count how many integers we can successfully parse
    while (fscanf(file, "%d", &num) == 1) {
        count++;
    }

    // Reset file position to beginning
    rewind(file);
    return count;
}

int* readIntegers(FILE * file, int* count) {
    // First, count the integers in the file
    *count = countIntegers(file);

    if (*count == 0) {
        fprintf(stderr, "No integers found in file\n");
        return NULL;
    }

    // Log the count for debugging
    fprintf(stderr, "Found %d integers in the file\n", *count);

    // Calculate memory needed with different methods
    size_t basic_size = (*count) * sizeof(int);
    size_t aligned_size = (((*count + 7) & ~7) * sizeof(int));

    fprintf(stderr, "Attempting to allocate: %zu bytes (basic) or %zu bytes (aligned)\n",
        basic_size, aligned_size);

    // Try standard malloc first
    fprintf(stderr, "Trying standard malloc...\n");
    int* array = (int*)malloc(basic_size);

    if (array == NULL) {
        fprintf(stderr, "Standard malloc failed for %zu bytes\n", basic_size);

        // Try aligned_alloc with a smaller alignment (e.g., 16 bytes instead of 64)
        fprintf(stderr, "Trying aligned_alloc with 16-byte alignment...\n");
        array = (int*)aligned_alloc(16, (((*count + 3) & ~3) * sizeof(int)));

        if (array == NULL) {
            fprintf(stderr, "All memory allocation attempts failed\n");

            // Get and print system memory info if possible
            fprintf(stderr, "Current memory state:\n");
            system("free -m");

            return NULL;
        }
        else {
            fprintf(stderr, "aligned_alloc with 16-byte alignment succeeded\n");
        }
    }
    else {
        fprintf(stderr, "Standard malloc succeeded\n");
    }

    // Reset file position to beginning
    rewind(file);

    // Read integers one by one
    int index = 0;
    int num;

    // Simple integer parsing - read until EOF or count is reached
    while (index < *count && fscanf(file, "%d", &num) == 1) {
        array[index++] = num;

        // Skip any non-digit characters (spaces, newlines, etc.)
        int ch;
        while ((ch = fgetc(file)) != EOF && !isdigit(ch) && ch != '-' && ch != '+') {
            // Just skip
        }

        // If we found a digit or sign, push it back
        if (ch != EOF) {
            ungetc(ch, file);
        }
    }

    // Update the actual count
    *count = index;

    // Debugging output
    fprintf(stderr, "Successfully read %d integers\n", index);

    if (index == 0) {
        free(array);
        return NULL;
    }

    return array;
}

// Write integers to a file
void writeIntegers(FILE * file, int array[], int count) {
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
