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

// Format time into appropriate units (ns, μs, ms, s)
void format_time(double time_seconds, char* buffer, size_t buffer_size);

// Create directory if it doesn't exist
int create_directory(const char* path);

// Function to extract filename from path
const char* get_filename(const char* path);

// Simple hash function (djb2)
uint64_t hash_string(const char* str);

#endif // COMMON_H
