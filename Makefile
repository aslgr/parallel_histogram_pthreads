CC = gcc
CFLAGS = -O3 -Wall -Wextra -pthread -Iinclude -pthread

TARGET = parallel_histogram

SRC = src/main.c src/data_generation.c src/histogram.c src/build_limits.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)