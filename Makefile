CC = clang
CFLAGS = -Wall -std=c99
TARGET = hash_benchmark
SOURCE = hash_benchmark.c

# Default build with LLVM
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -O3 -o $(TARGET) $(SOURCE)

run: $(TARGET)
	./$(TARGET)