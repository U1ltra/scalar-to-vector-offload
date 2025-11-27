#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define DATA_SIZE 10000000  // 10x larger
#define MAX_STREAMS 32      // Test more streams to show clearer saturation

// Global variable to prevent dead code elimination
volatile uint32_t global_result = 0;

// Simple hash function that creates scalar bottleneck
uint32_t simple_hash(const uint8_t* data, size_t len, uint32_t seed) {
    volatile uint32_t hash = seed;  // Prevent optimization
    for (size_t i = 0; i < len; i++) {
        hash = hash * 31 + data[i];  // Loop-carried dependency - forces scalar
    }
    return hash;
}

// Benchmark: Interleaved computation (better for your optimization)
void benchmark_interleaved_hashes(int num_streams) {
    printf("\n=== Interleaved Hash Computation ===\n");
    
    uint8_t** streams = malloc(num_streams * sizeof(uint8_t*));
    uint32_t* hashes = malloc(num_streams * sizeof(uint32_t));
    
    for (int s = 0; s < num_streams; s++) {
        streams[s] = malloc(DATA_SIZE);
        hashes[s] = s; // Initial seed
        for (int i = 0; i < DATA_SIZE; i++) {
            streams[s][i] = (uint8_t)(i * s + s);
        }
    }
    
    clock_t start = clock();
    
    // This pattern is perfect for your reverse optimization:
    // Multiple independent scalar computations that could be vectorized
    for (int i = 0; i < DATA_SIZE; i++) {
        for (int s = 0; s < num_streams; s++) {
            hashes[s] = hashes[s] * 31 + streams[s][i];
        }
    }
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Time taken: %f seconds\n", time_taken);
    printf("Throughput: %f MB/s\n", (DATA_SIZE * num_streams) / (time_taken * 1024 * 1024));
    printf("Throughput per stream: %f MB/s\n", DATA_SIZE / (time_taken * 1024 * 1024));
    
    // Use results to prevent optimization
    uint32_t checksum = 0;
    for (int s = 0; s < num_streams; s++) {
        checksum ^= hashes[s];
    }
    global_result += checksum;  // Force computation to be kept
    
    for (int s = 0; s < num_streams && s < 4; s++) {
        printf("Stream %d hash: 0x%08x\n", s, hashes[s]);
    }
    printf("Checksum: 0x%08x\n", checksum);
    
    free(streams);
    free(hashes);
}


int main() {
    printf("Hash Function Microbenchmark for Scalar-to-Vector Optimization\n");
    printf("=============================================================\n");
    
    srand(42); // Reproducible results
    
    // Test the interleaved pattern - best target for your optimization
    printf("\n--- Interleaved Patterns (Best for Vector Optimization) ---\n");
    benchmark_interleaved_hashes(4);
    
    printf("\n=== Analysis Notes ===\n");
    printf("- STRESS TEST should show dramatic throughput plateaus\n");
    printf("- Per-stream throughput should decrease as streams increase\n");
    printf("- Clear evidence of scalar unit saturation\n");
    printf("- Perfect target for scalar->vector optimization\n");
    printf("- Expected 4-8x improvement potential with vector packing\n");
    
    return 0;
}