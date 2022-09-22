#ifndef __UTILS_H__
#define __UTILS_H__

#include <map>
#include <stdint.h>
#include <stdlib.h>
#include <string>

// learned from lmbench lat_mem_rd
#define FIVE(X) X X X X X
#define TEN(X) FIVE(X) FIVE(X)
#define FIFTY(X) TEN(X) TEN(X) TEN(X) TEN(X) TEN(X)
#define HUNDRED(X) FIFTY(X) FIFTY(X)

// utilities
std::map<const char *, size_t> get_cache_sizes();
char **generate_random_pointer_chasing(size_t size);

// get time or cycles
// unit: ns or cycle
uint64_t get_time();
// prioritize cycle over time
void setup_time_or_cycles();
uint64_t get_time_or_cycles();

void bind_to_core();

// perf related
#ifdef __linux__
void setup_perf_cycles();
uint64_t perf_read_cycles();
void setup_perf_llc_misses();
uint64_t perf_read_llc_misses();
void setup_perf_llc_loads();
uint64_t perf_read_llc_loads();
#endif

#endif