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

// Read integers from a file using a chunked approach for large files
int* readIntegersInChunks(FILE * file, int* count) {
    // Initial buffer size for reading integers
    const int INITIAL_CAPACITY = 1024;
    const int CHUNK_GROWTH_FACTOR = 2;

    // Align to cache line boundary for better memory access
    int* array = (int*)aligned_alloc(CACHE_LINE_SIZE, INITIAL_CAPACITY * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    int capacity = INITIAL_CAPACITY;
    int index = 0;

    // Large buffer for reading lines
    char buffer[4096];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char* token = strtok(buffer, " \t\n,;");
        while (token != NULL) {
            // Dynamically resize array if needed
            if (index >= capacity) {
                capacity *= CHUNK_GROWTH_FACTOR;
                int* new_array = (int*)realloc(array, capacity * sizeof(int));

                if (new_array == NULL) {
                    fprintf(stderr, "Memory reallocation failed\n");
                    free(array);
                    return NULL;
                }
                array = new_array;
            }

            // Robust integer conversion
            char* endptr;
            long val = strtol(token, &endptr, 10);

            // Only add valid integers
            if (endptr != token && val >= INT_MIN && val <= INT_MAX) {
                array[index++] = (int)val;
            }

            token = strtok(NULL, " \t\n,;");
        }
    }

    // Trim excess memory if significantly larger
    if (capacity > index * 2) {
        int* trimmed_array = (int*)realloc(array, index * sizeof(int));
        if (trimmed_array != NULL) {
            array = trimmed_array;
        }
    }

    *count = index;
    return array;
}
