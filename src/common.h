// File: src/common.h
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>  // For isdigit and similar functions
#include <limits.h> // For INT_MIN and INT_MAX

// Define cache line size for optimizations
#define CACHE_LINE_SIZE 64

// Format time into appropriate units (ns, μs, ms, s)
void format_time(double time_seconds, char* buffer, size_t buffer_size);

// Create directory if it doesn't exist
int create_directory(const char* path);

// Function to extract filename from path
const char* get_filename(const char* path);

// Simple hash function (djb2)
uint64_t hash_string(const char* str);

// Read integers from a file into an array - chunk-based approach
int* readIntegersInChunks(FILE* file, int* count);

// Write integers to a file
void writeIntegers(FILE* file, int array[], int count);

#endif // COMMON_H
