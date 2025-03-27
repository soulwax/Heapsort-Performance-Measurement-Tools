CC = gcc
CFLAGS = -g -Wall -O2 -arch arm64
TARGET = bin/heapsort

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)

# usage: 
# compile: make
# run: ./heapsort <arguements being e.g unsorted numbers>
# clean: make clean