#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(int a[], int n, int i)
{
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && a[largest] < a[left])
        largest = left;

    if (right < n && a[largest] < a[right])
        largest = right;

    if (largest != i)
    {
        swap(&a[i], &a[largest]);
        heapify(a, n, largest);
    }
}

void buildMaxHeap(int a[], int n)
{
    int i;
    for (i = n / 2 - 1; i >= 0; i--)
        heapify(a, n, i);
}

void heapSort(int a[], int n)
{
    int i;
    buildMaxHeap(a, n);
    for (i = n - 1; i >= 0; i--)
    {
        swap(&a[0], &a[i]);
        heapify(a, i, 0);
    }
}

// Count the number of integers in a file
int countIntegers(FILE *file)
{
    int count = 0;
    char buffer[1024];

    // Reset file position to the beginning
    rewind(file);

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        char *token = strtok(buffer, " \t\n,;");
        while (token != NULL)
        {
            // Check if token is a valid integer
            int isValid = 1;
            for (int i = 0; token[i] != '\0'; i++)
            {
                if (i == 0 && (token[i] == '-' || token[i] == '+'))
                    continue;
                if (!isdigit(token[i]))
                {
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
int *readIntegers(FILE *file, int *count)
{
    *count = countIntegers(file);

    if (*count == 0)
        return NULL;

    int *array = (int *)malloc(*count * sizeof(int));
    if (array == NULL)
        return NULL;

    int index = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file) != NULL && index < *count)
    {
        char *token = strtok(buffer, " \t\n,;");
        while (token != NULL && index < *count)
        {
            // Check if token is a valid integer
            int isValid = 1;
            for (int i = 0; token[i] != '\0'; i++)
            {
                if (i == 0 && (token[i] == '-' || token[i] == '+'))
                    continue;
                if (!isdigit(token[i]))
                {
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
void writeIntegers(FILE *file, int array[], int count)
{
    for (int i = 0; i < count; i++)
    {
        fprintf(file, "%d", array[i]);
        if (i < count - 1)
            fprintf(file, " ");
    }
    fprintf(file, "\n");
}

void printUsage(char *programName)
{
    printf("Usage:\n");
    printf("  %s <num1> <num2> <num3> ...          # Sort numbers from command line\n", programName);
    printf("  %s -f <input_file>                   # Sort numbers from input file\n", programName);
    printf("  %s -f <input_file> -o <output_file>  # Sort numbers from input file and write to output file\n", programName);
}

int main(int argc, char *argv[])
{
    int *a = NULL;
    int n = 0;
    FILE *inputFile = NULL;
    FILE *outputFile = NULL;
    int usingFiles = 0;

    // Parse command line arguments
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    // Check if using file input
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
        {
            inputFile = fopen(argv[i + 1], "r");
            if (inputFile == NULL)
            {
                printf("Error: Could not open input file '%s'\n", argv[i + 1]);
                return 1;
            }
            usingFiles = 1;

            // Check for output file
            if (i + 2 < argc && strcmp(argv[i + 2], "-o") == 0 && i + 3 < argc)
            {
                outputFile = fopen(argv[i + 3], "w");
                if (outputFile == NULL)
                {
                    printf("Error: Could not open output file '%s'\n", argv[i + 3]);
                    fclose(inputFile);
                    return 1;
                }
            }
            break;
        }
    }

    if (usingFiles)
    {
        // Read integers from file
        a = readIntegers(inputFile, &n);
        fclose(inputFile);

        if (a == NULL || n == 0)
        {
            printf("Error: No valid integers found in the input file\n");
            return 1;
        }
    }
    else
    {
        // Use command line arguments as before
        n = argc - 1;
        a = (int *)malloc(n * sizeof(int));

        if (a == NULL)
        {
            printf("Memory allocation failed\n");
            return 1;
        }

        for (int i = 0; i < n; i++)
        {
            a[i] = atoi(argv[i + 1]);
        }
    }

    // Print original array
    printf("Original array: ");
    for (int i = 0; i < n; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");

    // Sort the array
    heapSort(a, n);

    // Print sorted array
    printf("Sorted array: ");
    for (int i = 0; i < n; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");

    // Write to output file if specified
    if (outputFile != NULL)
    {
        writeIntegers(outputFile, a, n);
        fclose(outputFile);
        printf("Sorted array written to output file\n");
    }

    free(a);
    return 0;
}