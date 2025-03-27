# Heapsort + Random Number Generator into File

## Basic Makefile Commands

1. **Build all programs**:

   ```sh
   make
   ```

   This will compile both the `heapsort` program (from main.c) and the `gen_randf` program (from gen_randf.c) and place the executables in the `bin` directory.

2. **Build specific programs**:

   ```sh
   make heapsort
   ```

   or

   ```sh
   make gen_randf
   ```

   These commands build only the specified program.

3. **Clean the build**:

   ```sh
   make clean
   ```

   This removes the `bin` directory and all compiled executables.

## Using the Programs

### For heapsort

1. **Sort numbers directly from command line**:

   ```sh
   ./bin/heapsort 5 2 9 1 7 4
   ```

   This sorts the numbers 5, 2, 9, 1, 7, 4 directly.

2. **Sort numbers from an input file**:

   ```sh
   ./bin/heapsort -f input/randnum_ea6c2b8b5c51682f.txt
   ```

   This reads integers from the specified file and sorts them. Make sure to provide the exact path to the file.

3. **Sort and save to output file**:

   ```sh
   ./bin/heapsort -f input/randnum_ea6c2b8b5c51682f.txt -o sorted.txt
   ```

   This reads from the input file, sorts the numbers, and writes the result to `sorted.txt`.

### For gen_randf

1. **Generate default random numbers** (100 numbers between 1-1000):

   ```sh
   ./bin/gen_randf
   ```

2. **Generate a specific number of random values**:

   ```sh
   ./bin/gen_randf -c 500
   ```

   This generates 500 random numbers.

3. **Specify range for random numbers**:

   ```sh
   ./bin/gen_randf -c 200 -min -100 -max 100
   ```

   This generates 200 random numbers between -100 and 100.

## Makefile Configuration

The Makefile includes some important configurations:

- `CC = gcc`: Compiler to use
- `CFLAGS = -g -Wall -O2 -arch arm64`: Compiler flags
  - `-g`: Include debugging symbols
  - `-Wall`: Show all warnings
  - `-O2`: Optimization level 2
  - `-arch arm64`: Compile for ARM64 architecture (for M1/M2 Macs)

If you're not on an ARM-based Mac, you might want to remove the `-arch arm64` flag or change it to match your architecture.

## Debugging Your Issue

Based on your original problem with the file `input/randnum_ea6c2b8b5c51682f.txt`, make sure:

1. The file path is correct relative to where you're running the command
2. The `input` directory exists in your current working directory
3. The file contains properly formatted integers

If the issue persists, you might want to modify the `main.c` file to add more debugging output, particularly in the `readIntegers` and `countIntegers` functions.
