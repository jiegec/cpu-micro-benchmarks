// reference:
// https://github.com/intel/lmbench/blob/master/src/lat_mem_rd.c

#include "include/utils.h"
#include <algorithm>
#include <assert.h>
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

FILE *fp;

// measure memory latency with pointer chasing
void test(int size, int warmup, int iterations) {
  char **buffer = generate_random_pointer_chasing(size);
  if (!buffer) {
    // skipped
    return;
  }

  char **p = (char **)buffer[0];

  // warmup
  for (int i = 0; i < warmup; i++) {
    HUNDRED(ONE);
  }

  // benchmark
  uint64_t before = get_time();

  for (int i = 0; i < iterations; i++) {
    HUNDRED(ONE);
  }

  // avoid optimization
  *(volatile char *)*p;
  uint64_t after = get_time();
  fprintf(fp, "%d,%.2f\n", size, (double)(after - before) / iterations / 100);
  fflush(fp);

  delete[] buffer;
}

int main(int argc, char *argv[]) {
  fp = fopen("memory_latency.csv", "w");
  assert(fp);

  int warmup = 50000;
  int iteration = 500000;

  int opt;
  while ((opt = getopt(argc, argv, "w:i:")) != -1) {
    switch (opt) {
    case 'w':
      sscanf(optarg, "%d", &warmup);
      break;
    case 'i':
      sscanf(optarg, "%d", &iteration);
      break;
    default:
      fprintf(stderr, "Usage: %s [-w warmup] [-i iteration]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  std::map<const char *, size_t> cache_sizes = get_cache_sizes();
  for (auto it : cache_sizes) {
    const char *name = it.first;
    size_t size = it.second;
    if (size != 0) {
      fprintf(fp, "%s cache: %zu bytes\n", name, size);
    }
  }

  fprintf(fp, "size,time(ns)\n");
  test(1024, warmup, iteration);
  test(1024 * 2, warmup, iteration);
  test(1024 * 4, warmup, iteration);
  test(1024 * 16, warmup, iteration);
  test(1024 * 24, warmup, iteration);
  test(1024 * 32, warmup, iteration);
  test(1024 * 28, warmup, iteration);
  test(1024 * 36, warmup, iteration);
  test(1024 * 40, warmup, iteration);
  test(1024 * 44, warmup, iteration);
  test(1024 * 48, warmup, iteration);
  test(1024 * 64, warmup, iteration);
  test(1024 * 80, warmup, iteration);
  test(1024 * 96, warmup, iteration);
  test(1024 * 112, warmup, iteration);
  test(1024 * 128, warmup, iteration);
  test(1024 * 144, warmup, iteration);
  test(1024 * 160, warmup, iteration);
  test(1024 * 176, warmup, iteration);
  test(1024 * 192, warmup, iteration);
  test(1024 * 208, warmup, iteration);
  test(1024 * 216, warmup, iteration);
  test(1024 * 224, warmup, iteration);
  test(1024 * 232, warmup, iteration);
  test(1024 * 240, warmup, iteration);
  test(1024 * 256, warmup, iteration);
  test(1024 * 288, warmup, iteration);
  test(1024 * 320, warmup, iteration);
  test(1024 * 352, warmup, iteration);
  test(1024 * 384, warmup, iteration);
  test(1024 * 512, warmup, iteration);
  test(1024 * 768, warmup, iteration);
  test(1024 * 1024, warmup, iteration);
  test(1024 * 1024 * 2, warmup, iteration);
  test(1024 * 1024 * 4, warmup, iteration);
  test(1024 * 1024 * 8, warmup, iteration);
  test(1024 * 1024 * 16, warmup, iteration);
  test(1024 * 1024 * 32, warmup, iteration);
  test(1024 * 1024 * 48, warmup, iteration);
  test(1024 * 1024 * 64, warmup, iteration);
  test(1024 * 1024 * 128, warmup, iteration);
  test(1024 * 1024 * 192, warmup, iteration);
  test(1024 * 1024 * 256, warmup, iteration);
  test(1024 * 1024 * 384, warmup, iteration);
  test(1024 * 1024 * 512, warmup, iteration);
  test(1024 * 1024 * 1024, warmup, iteration);
  printf("Results are written to memory_latency.csv\n");
  return 0;
}
