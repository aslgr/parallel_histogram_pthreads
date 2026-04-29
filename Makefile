CC = gcc
CFLAGS = -O3 -Wall -Wextra -pthread

TARGET = parallel_histogram
SRC = src/parallel_histogram.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)