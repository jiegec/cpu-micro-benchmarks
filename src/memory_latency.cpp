// reference:
// https://github.com/intel/lmbench/blob/master/src/lat_mem_rd.c

#include "include/utils.h"
#include <algorithm>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

// learned from lmbench lat_mem_rd
#define ONE p = (char **)*p;

// measure memory latency with pointer chasing
void test(int size) {
  char **buffer = generate_random_pointer_chasing(size);

  const int warmup = 50000;
  const int iterations = 500000;
  char **p = (char **)buffer[0];

  // warmup
  for (int i = 0; i < warmup; i++) {
    HUNDRED(ONE);
  }

  // benchmark
  uint64_t before = get_time_ns();

  for (int i = 0; i < iterations; i++) {
    HUNDRED(ONE);
  }

  // avoid optimization
  *(volatile char *)*p;
  uint64_t after = get_time_ns();
  printf("%d,%.2f\n", size, (double)(after - before) / iterations / 100);
  fflush(stdout);

  delete[] buffer;
}

int main(int argc, char *argv[]) {
  std::map<const char *, size_t> cache_sizes = get_cache_sizes();
  for (auto it : cache_sizes) {
    const char *name = it.first;
    size_t size = it.second;
    if (size != 0) {
      printf("%s cache: %zu bytes\n", name, size);
    }
  }

  printf("size,time(ns)\n");
  test(1024);
  test(1024 * 2);
  test(1024 * 4);
  test(1024 * 16);
  test(1024 * 24);
  test(1024 * 32);
  test(1024 * 28);
  test(1024 * 36);
  test(1024 * 40);
  test(1024 * 44);
  test(1024 * 48);
  test(1024 * 64);
  test(1024 * 96);
  test(1024 * 128);
  test(1024 * 192);
  test(1024 * 224);
  test(1024 * 256);
  test(1024 * 288);
  test(1024 * 384);
  test(1024 * 512);
  test(1024 * 768);
  test(1024 * 1024);
  test(1024 * 1024 * 2);
  test(1024 * 1024 * 4);
  test(1024 * 1024 * 8);
  test(1024 * 1024 * 16);
  test(1024 * 1024 * 32);
  test(1024 * 1024 * 48);
  test(1024 * 1024 * 64);
  test(1024 * 1024 * 128);
  test(1024 * 1024 * 192);
  test(1024 * 1024 * 256);
  test(1024 * 1024 * 384);
  test(1024 * 1024 * 512);
  test(1024 * 1024 * 1024);
  return 0;
}