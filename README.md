# Sorting Algorithm Benchmark Suite

A performance benchmarking suite for comparing sorting algorithms, specifically optimized implementations of HeapSort and QuickSort.

## Features

- **Multiple Sorting Algorithms**
  - HeapSort: Optimized hybrid heap sort with insertion sort for small arrays
  - QuickSort: Cache-aware implementation with median-of-three pivot selection

- **Random Number Generator**
  - Generate test data sets of arbitrary size
  - Configurable value ranges
  - Hash-based file naming for tracking and repeatability

- **Comprehensive Benchmarking**
  - Single algorithm performance analysis
  - Comparative analysis between algorithms
  - Configurable size ranges and steps
  - Multiple repetitions for statistical validity

- **Performance Visualization**
  - Generate plots showing sorting time vs. array size
  - Logarithmic scale analysis for complexity validation
  - Algorithm comparison charts

## Building

```bash
# Build all components
make

# Build specific components
make heapsort
make quicksort
make genrand_f
make benchmark
```

## Usage Examples

### Generating Random Numbers

```bash
# Generate 100 random numbers (default: 1-1000 range)
./bin/genrand_f

# Generate 500 random numbers
./bin/genrand_f -c 500

# Generate 200 random numbers between -100 and 100
./bin/genrand_f -c 200 -min -100 -max 100
```

### Sorting Numbers

```bash
# Sort numbers from command line
./bin/heapsort 5 2 9 1 7 4
./bin/quicksort 5 2 9 1 7 4

# Sort numbers from input file
./bin/heapsort -f input/randnum_fixed.txt
./bin/quicksort -f input/randnum_fixed.txt

# Sort and write to specified output file
./bin/heapsort -f input/randnum_fixed.txt -o sorted_output.txt
./bin/quicksort -f input/randnum_fixed.txt -o sorted_output.txt

# Use cache-optimized block sorting (heapsort only)
./bin/heapsort -f input/randnum_fixed.txt --block-sort
```

### Running Benchmarks

```bash
# Run benchmark with default settings (heapsort, 1000-100000 elements)
make run-benchmark

# Run custom benchmark
make run-custom-benchmark

# Run algorithm comparison (heapsort vs quicksort)
make run-comparison-benchmark

# Custom benchmark with specific options
./bin/benchmark --min 500 --max 20000 --step 500 --repeats 5

# Benchmark specific algorithm
./bin/benchmark --algorithm heap
./bin/benchmark --algorithm quick

# Compare both algorithms
./bin/benchmark --algorithm both
# or
./bin/benchmark --algorithm-compare
```

### Visualizing Results

```bash
# Visualize the most recent benchmark results
python3 visualize_benchmark.py

# Visualize specific benchmark results
python3 visualize_benchmark.py benchmark_results/heapsort_benchmark_1000_100000.csv

# Compare algorithm results
python3 visualize_benchmark.py --compare benchmark_results/algorithm_comparison_1000_100000.csv
```

## Algorithm Details

### HeapSort Implementation

The HeapSort implementation features:

- Hybrid approach with insertion sort for small arrays
- Cache-friendly memory access patterns
- Compiler hints for branch prediction
- Block-based variant for improved cache behavior

### QuickSort Implementation

The QuickSort implementation features:

- Median-of-three pivot selection for improved partitioning
- Cache-friendly memory access with prefetching
- Tail-recursive implementation to reduce stack space
- Hybrid approach with insertion sort for small partitions

## Performance Analysis

The benchmark suite measures algorithm performance across different array sizes and provides insights into:

- Raw sorting speed (time per N elements)
- Algorithmic complexity (via log-log plots)
- Comparative efficiency between algorithms
- Memory access patterns and cache effects

## Project Structure

```plaintext
.
├── bin/              # Compiled executables
├── src/              # Source code
│   ├── benchmark.c   # Benchmarking tool
│   ├── common.c      # Common utility functions
│   ├── common.h      # Common header
│   ├── genrand_f.c   # Random number generator
│   ├── heapsort_f.c  # HeapSort implementation
│   └── quicksort_f.c # QuickSort implementation
├── input/            # Input data files
├── output/           # Sorted output files
├── benchmark_results/# CSV benchmark results
├── benchmark_plots/  # Generated visualizations
└── Makefile          # Build configuration
```

## Requirements

- C compiler with C11 support
- Python 3.x with matplotlib and pandas (for visualization)
- POSIX-compliant operating system

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
