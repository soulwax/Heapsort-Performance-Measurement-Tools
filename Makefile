# File: Makefile

CC = gcc
CFLAGS = -g -Wall -O2 -arch arm64
BIN_DIR = bin
SRC_DIR = src
TARGETS = heapsort gen_randf

.PHONY: all clean directories

all: directories $(TARGETS)

directories:
	mkdir -p $(BIN_DIR)

heapsort: $(SRC_DIR)/heapsort_f.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $<
	@echo "Built $@ in $(BIN_DIR)/"

gen_randf: $(SRC_DIR)/genrand_f.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $<
	@echo "Built $@ in $(BIN_DIR)/"

clean:
	rm -rf $(BIN_DIR)

# Usage examples:
# Compile all:
#   make
#
# Compile specific target:
#   make heapsort
#   make gen_randf
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
# Run gen_randf with default settings (100 numbers between 1-1000):
#   ./bin/gen_randf
#
# Generate 500 random numbers:
#   ./bin/gen_randf -c 500
#
# Generate 200 random numbers between -100 and 100:
#   ./bin/gen_randf -c 200 -min -100 -max 100
#
# Clean:
#   make clean
