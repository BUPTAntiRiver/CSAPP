#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

// Cache line structure
typedef struct {
    int valid;           // Valid bit
    unsigned long tag;   // Tag bits
    int lru_counter;     // For LRU replacement policy
} cache_line_t;

// Cache set structure
typedef struct {
    cache_line_t *lines; // Array of cache lines
} cache_set_t;

// Cache structure
typedef struct {
    cache_set_t *sets;   // Array of cache sets
    int S;               // Number of sets (2^s)
    int E;               // Associativity (lines per set)
    int B;               // Block size (2^b)
    int s, b;            // Number of set and block bits
} cache_t;

struct csim
{
    /* cache simulation structure */
    int cache_size;
    int block_size;
    int associativity;
    int hits;
    int misses;
    int evictions;
};

struct csim cache_simulation = {
    .cache_size = 1024,
    .block_size = 64,
    .associativity = 4,
    .hits = 0,
    .misses = 0,
    .evictions = 0
};

cache_t cache;
int lru_time = 0; // Global LRU counter

// Initialize cache
void initCache(int s, int E, int b) {
    cache.S = 1 << s;
    cache.E = E;
    cache.B = 1 << b;
    cache.s = s;
    cache.b = b;
    
    cache.sets = malloc(cache.S * sizeof(cache_set_t));
    for (int i = 0; i < cache.S; i++) {
        cache.sets[i].lines = malloc(E * sizeof(cache_line_t));
        for (int j = 0; j < E; j++) {
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].tag = 0;
            cache.sets[i].lines[j].lru_counter = 0;
        }
    }
}

// Simulate cache access
void accessCache(unsigned long address, int verbose) {
    // Extract set index and tag from address
    unsigned long set_index = (address >> cache.b) & ((1 << cache.s) - 1);
    unsigned long tag = address >> (cache.s + cache.b);
    
    cache_set_t *set = &cache.sets[set_index];
    int empty_line = -1;
    int lru_line = 0;
    
    // Check for hit
    for (int i = 0; i < cache.E; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            // Cache hit
            set->lines[i].lru_counter = lru_time++;
            cache_simulation.hits++;
            if (verbose) printf("hit ");
            return;
        }
        
        // Track empty line and LRU line
        if (!set->lines[i].valid && empty_line == -1) {
            empty_line = i;
        }
        if (set->lines[i].lru_counter < set->lines[lru_line].lru_counter) {
            lru_line = i;
        }
    }
    
    // Cache miss
    cache_simulation.misses++;
    if (verbose) printf("miss ");
    
    // Find line to replace
    int replace_line;
    if (empty_line != -1) {
        // Use empty line
        replace_line = empty_line;
    } else {
        // Evict LRU line
        replace_line = lru_line;
        cache_simulation.evictions++;
        if (verbose) printf("eviction ");
    }
    
    // Update cache line
    set->lines[replace_line].valid = 1;
    set->lines[replace_line].tag = tag;
    set->lines[replace_line].lru_counter = lru_time++;
}

void processTrace(char *tracefile, int verbose, int s, int E, int b) {
    initCache(s, E, b);
    
    FILE *fp = fopen(tracefile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open trace file %s\n", tracefile);
        exit(1);
    }
    
    char operation;
    unsigned long address;
    int size;
    
    // Read each line from the trace file
    while (fscanf(fp, " %c %lx,%d", &operation, &address, &size) == 3) {
        if (operation == 'I') {
            continue; // Skip instruction loads
        }
        
        if (verbose) {
            printf("%c %lx,%d ", operation, address, size);
        }
        
        switch (operation) {
            case 'L': // Load
            case 'S': // Store
                accessCache(address, verbose);
                break;
            case 'M': // Modify (load + store)
                accessCache(address, verbose); // Load
                accessCache(address, verbose); // Store
                break;
        }
        
        if (verbose) {
            printf("\n");
        }
    }
    
    fclose(fp);
}

int main(int argc, char *argv[])
{
    int s = 0; // number of set index bits
    int E = 0; // associativity (number of lines per set)
    int b = 0; // number of block bits
    char *tracefile = NULL;
    int verbose = 0;
    int opt;
    
    // Parse command line arguments
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                exit(1);
        }
    }
    
    // Process the trace file
    if (tracefile != NULL) {
        processTrace(tracefile, verbose, s, E, b);
    }
    
    printSummary(cache_simulation.hits, cache_simulation.misses, cache_simulation.evictions);
    return 0;
}
