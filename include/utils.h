#ifndef __UTILS_H__
#define __UTILS_H__

#include <map>
#include <stdlib.h>
#include <string>
#include <stdint.h>

std::map<const char *, size_t> get_cache_sizes();
uint64_t get_time_ns();

#endif