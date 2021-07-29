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
uint64_t get_time_ns();
char **generate_random_pointer_chasing(size_t size);

#endif