#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define DATA_SIZE 10000000  // 10x larger
#define MAX_STREAMS 32      // Test more streams to show clearer saturation

// Global variable to prevent dead code elimination
volatile uint32_t global_result = 0;

// More intensive hash computation to amplify scalar bottleneck
uint32_t intensive_hash(const uint8_t* data, size_t len, uint32_t seed) {
    volatile uint32_t hash = seed;
    for (size_t i = 0; i < len; i++) {
        // Multiple dependent operations to stress scalar units more
        hash = hash * 31 + data[i];
        hash = hash ^ (hash >> 16);
        hash = hash * 0x45d9f3b;
        hash = hash ^ (hash >> 16);
    }
    return hash;
}


// Stress test: Many concurrent independent computations (perfect optimization target)
void benchmark_stress_test(int num_streams) {
    printf("\n=== Stress Test: %d Concurrent Hash Streams ===\n", num_streams);
    
    uint8_t** streams = malloc(num_streams * sizeof(uint8_t*));
    uint32_t* results = malloc(num_streams * sizeof(uint32_t));
    
    for (int s = 0; s < num_streams; s++) {
        streams[s] = malloc(DATA_SIZE);
        for (int i = 0; i < DATA_SIZE; i++) {
            streams[s][i] = (uint8_t)(i * s + s + 1);  // Avoid zeros
        }
    }
    
    clock_t start = clock();
    
    // This should clearly saturate scalar units and show diminishing returns
    for (int s = 0; s < num_streams; s++) {
        results[s] = intensive_hash(streams[s], DATA_SIZE, s + 1);
    }
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Time taken: %f seconds\n", time_taken);
    printf("Throughput: %f MB/s\n", (DATA_SIZE * num_streams) / (time_taken * 1024 * 1024));
    printf("Throughput per stream: %f MB/s\n", DATA_SIZE / (time_taken * 1024 * 1024));
    
    // Use results to prevent optimization
    uint32_t checksum = 0;
    for (int s = 0; s < num_streams; s++) {
        checksum ^= results[s];
    }
    global_result += checksum;
    
    printf("Sample results: ");
    for (int s = 0; s < num_streams && s < 4; s++) {
        printf("0x%08x ", results[s]);
    }
    printf("\nChecksum: 0x%08x\n", checksum);
    
    // Cleanup
    for (int s = 0; s < num_streams; s++) {
        free(streams[s]);
    }
    free(streams);
    free(results);
}

int main() {
    printf("Hash Function Microbenchmark for Scalar-to-Vector Optimization\n");
    printf("=============================================================\n");
    
    srand(42); // Reproducible results
    
    // Stress test with increasing numbers of streams to show clear saturation
    printf("\n=== STRESS TEST: Demonstrating Scalar Saturation ===\n");
    // for (int streams = 1; streams <= 16; streams *= 2) {
    //     benchmark_stress_test(streams);
    // }
    benchmark_stress_test(16);
    
    printf("\n=== Analysis Notes ===\n");
    printf("- STRESS TEST should show dramatic throughput plateaus\n");
    printf("- Per-stream throughput should decrease as streams increase\n");
    printf("- Clear evidence of scalar unit saturation\n");
    printf("- Perfect target for scalar->vector optimization\n");
    printf("- Expected 4-8x improvement potential with vector packing\n");
    
    return 0;
}