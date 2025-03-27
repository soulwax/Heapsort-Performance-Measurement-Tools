#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

// Function to create a directory if it doesn't exist
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

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(int a[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && a[largest] < a[left])
        largest = left;

    if (right < n && a[largest] < a[right])
        largest = right;

    if (largest != i) {
        swap(&a[i], &a[largest]);
        heapify(a, n, largest);
    }
}

void buildMaxHeap(int a[], int n) {
    int i;
    for (i = n / 2 - 1; i >= 0; i--)
        heapify(a, n, i);
}

void heapSort(int a[], int n) {
    int i;
    buildMaxHeap(a, n);
    for (i = n - 1; i >= 0; i--) {
        swap(&a[0], &a[i]);
        heapify(a, i, 0);
    }
}

// Count the number of integers in a file
int countIntegers(FILE * file) {
    int count = 0;
    char buffer[1024];

    // Reset file position to the beginning
    rewind(file);

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char* token = strtok(buffer, " \t\n,;");
        while (token != NULL) {
            // Check if token is a valid integer
            int isValid = 1;
            for (int i = 0; token[i] != '\0'; i++) {
                if (i == 0 && (token[i] == '-' || token[i] == '+'))
                    continue;
                if (!isdigit(token[i])) {
                    isValid = 0;
                    break;
                }
            }
            if (isValid && strlen(token) > 0)
                count++;

            token = strtok(NULL, " \t\n,;");
        }
    }

    // Reset file position to the beginning
    rewind(file);
    return count;
}

// Read integers from a file into an array
int* readIntegers(FILE * file, int* count) {
    *count = countIntegers(file);

    if (*count == 0)
        return NULL;

    int* array = (int*)malloc(*count * sizeof(int));
    if (array == NULL)
        return NULL;

    int index = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file) != NULL && index < *count) {
        char* token = strtok(buffer, " \t\n,;");
        while (token != NULL && index < *count) {
            // Check if token is a valid integer
            int isValid = 1;
            for (int i = 0; token[i] != '\0'; i++) {
                if (i == 0 && (token[i] == '-' || token[i] == '+'))
                    continue;
                if (!isdigit(token[i])) {
                    isValid = 0;
                    break;
                }
            }

            if (isValid && strlen(token) > 0)
                array[index++] = atoi(token);

            token = strtok(NULL, " \t\n,;");
        }
    }

    return array;
}

// Write sorted integers to a file
void writeIntegers(FILE * file, int array[], int count) {
    for (int i = 0; i < count; i++) {
        fprintf(file, "%d", array[i]);
        if (i < count - 1)
            fprintf(file, " ");
    }
    fprintf(file, "\n");
}

void printUsage(char* programName) {
    printf("Usage:\n");
    printf("  %s <num1> <num2> <num3> ...          # Sort numbers from command line\n", programName);
    printf("  %s -f <input_file>                   # Sort numbers from input file\n", programName);
    printf("  %s -f <input_file> -o <output_file>  # Sort numbers from input file and write to output file\n", programName);
}

int main(int argc, char* argv[]) {
    int* a = NULL;
    int n = 0;
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    int usingFiles = 0;
    char* inputFilename = NULL;

    // Parse command line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
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
        a = (int*)malloc(n * sizeof(int));

        if (a == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }

        for (int i = 0; i < n; i++) {
            a[i] = atoi(argv[i + 1]);
        }
    }

    // Create a copy of the original array for displaying
    int* original = (int*)malloc(n * sizeof(int));
    if (original == NULL) {
        printf("Memory allocation failed\n");
        free(a);
        return 1;
    }
    memcpy(original, a, n * sizeof(int));

    // Sort the array
    heapSort(a, n);

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

        fclose(outputFile);
    }
    else {
        // Print to console if no output file specified
        printf("Original array: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", original[i]);
        }
        printf("\n");

        printf("Sorted array: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", a[i]);
        }
        printf("\n");
    }

    free(original);
    free(a);
    return 0;
}
