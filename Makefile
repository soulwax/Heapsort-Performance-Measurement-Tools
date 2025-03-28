# File: Makefile

CC = gcc
CFLAGS = -g -Wall -O2 -arch arm64 -Wl,-dead_strip
BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj
TARGETS = heapsort genrand_f benchmark
COMMON_OBJ = $(OBJ_DIR)/common.o

.PHONY: all clean directories

all: directories $(TARGETS)

directories:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(COMMON_OBJ): $(SRC_DIR)/common.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

heapsort: $(SRC_DIR)/heapsort_f.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^
	@echo "Built $@ in $(BIN_DIR)/"

genrand_f: $(SRC_DIR)/genrand_f.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^
	@echo "Built $@ in $(BIN_DIR)/"

benchmark: $(SRC_DIR)/benchmark.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^
	@echo "Built $@ in $(BIN_DIR)/"

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)
	rm -f .temp_sort_output .latest_file
	find input -type f -not -name "randnum_fixed.txt" -exec rm -f {} \;
	find output -type f -not -name "randnum_fixed.txt" -exec rm -f {} \;
	rm -rf benchmark_results
	rm -rf benchmark_plots

# Run benchmark with default settings
run-benchmark: benchmark heapsort genrand_f
	@echo "Running benchmark with default settings..."
	./$(BIN_DIR)/benchmark

# Run benchmark with custom settings (example)
run-custom-benchmark: benchmark heapsort genrand_f
	@echo "Running custom benchmark..."
	./$(BIN_DIR)/benchmark --min 500 --max 10000 --step 500 --repeats 5

# Usage examples:
# Compile all:
#   make
#
# Compile specific target:
#   make heapsort
#   make genrand_f
#   make benchmark
#
# Run heapsort with command line arguments:
#   ./bin/heapsort 5 2 9 1 7 4
#
# Run heapsort with input file:
#   ./bin/heapsort -f input.txt
#
# Run heapsort with input file and output file:
#   ./bin/heapsort -f input.txt -o sorted.txt
#
# Run genrand_f with default settings (100 numbers between 1-1000):
#   ./bin/genrand_f
#
# Generate 500 random numbers:
#   ./bin/genrand_f -c 500
#
# Generate 200 random numbers between -100 and 100:
#   ./bin/genrand_f -c 200 -min -100 -max 100
#
# Run benchmark with default settings:
#   make run-benchmark
#
# Run benchmark with custom settings:
#   make run-custom-benchmark
#
# Clean:
#   make clean
