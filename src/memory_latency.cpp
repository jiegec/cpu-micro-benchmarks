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
void test(int size, int warmup, int iterations, bool perf) {
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
#ifdef __linux__
  uint64_t cycles_before, llc_misses_before, llc_loads_before;
  if (perf) {
    cycles_before = perf_read_cycles();
    llc_misses_before = perf_read_llc_misses();
    llc_loads_before = perf_read_llc_loads();
  }
#endif

  for (int i = 0; i < iterations; i++) {
    HUNDRED(ONE);
  }

  // avoid optimization
  *(volatile char *)*p;
  uint64_t after = get_time();
#ifdef __linux__
  uint64_t cycles_after, llc_misses_after, llc_loads_after;
  if (perf) {
    cycles_after = perf_read_cycles();
    llc_misses_after = perf_read_llc_misses();
    llc_loads_after = perf_read_llc_loads();
  }
#endif

#ifdef __linux__
  if (perf) {
    fprintf(fp, "%d,%.2f,%.2f,%.2f,%.2f\n", size,
            (double)(after - before) / iterations / 100,
            (double)(cycles_after - cycles_before) / iterations / 100,
            (double)(llc_misses_after - llc_misses_before) / iterations / 100,
            (double)(llc_loads_after - llc_loads_before) / iterations / 100);
  } else {
    fprintf(fp, "%d,%.2f\n", size, (double)(after - before) / iterations / 100);
  }
#else
  fprintf(fp, "%d,%.2f\n", size, (double)(after - before) / iterations / 100);
#endif
  fflush(fp);

  delete[] buffer;
}

int main(int argc, char *argv[]) {
  fp = fopen("memory_latency.csv", "w");
  assert(fp);

  int warmup = 50000;
  int iteration = 500000;
  bool perf = false;

  int opt;
  while ((opt = getopt(argc, argv, "w:i:p")) != -1) {
    switch (opt) {
    case 'w':
      sscanf(optarg, "%d", &warmup);
      break;
    case 'i':
      sscanf(optarg, "%d", &iteration);
      break;
    case 'p':
      perf = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-w warmup] [-i iteration] [-p]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

#ifdef __linux__
  if (perf) {
    setup_perf_cycles();
    setup_perf_llc_misses();
    setup_perf_llc_loads();
  }
#endif

  std::map<const char *, size_t> cache_sizes = get_cache_sizes();
  for (auto it : cache_sizes) {
    const char *name = it.first;
    size_t size = it.second;
    if (size != 0) {
      fprintf(fp, "%s cache: %zu bytes\n", name, size);
    }
  }

#ifdef __linux__
  if (perf) {
    fprintf(fp, "size,time(ns),cycles,llc miss ratio,llc load ratio\n");
  } else {
    fprintf(fp, "size,time(ns)\n");
  }
#else
  fprintf(fp, "size,time(ns)\n");
#endif
  test(1024, warmup, iteration, perf);
  test(1024 * 2, warmup, iteration, perf);
  test(1024 * 4, warmup, iteration, perf);
  test(1024 * 16, warmup, iteration, perf);
  test(1024 * 24, warmup, iteration, perf);
  test(1024 * 32, warmup, iteration, perf);
  test(1024 * 28, warmup, iteration, perf);
  test(1024 * 36, warmup, iteration, perf);
  test(1024 * 40, warmup, iteration, perf);
  test(1024 * 44, warmup, iteration, perf);
  test(1024 * 48, warmup, iteration, perf);
  test(1024 * 64, warmup, iteration, perf);
  test(1024 * 80, warmup, iteration, perf);
  test(1024 * 96, warmup, iteration, perf);
  test(1024 * 112, warmup, iteration, perf);
  test(1024 * 128, warmup, iteration, perf);
  test(1024 * 144, warmup, iteration, perf);
  test(1024 * 160, warmup, iteration, perf);
  test(1024 * 176, warmup, iteration, perf);
  test(1024 * 192, warmup, iteration, perf);
  test(1024 * 208, warmup, iteration, perf);
  test(1024 * 216, warmup, iteration, perf);
  test(1024 * 224, warmup, iteration, perf);
  test(1024 * 232, warmup, iteration, perf);
  test(1024 * 240, warmup, iteration, perf);
  test(1024 * 256, warmup, iteration, perf);
  test(1024 * 288, warmup, iteration, perf);
  test(1024 * 320, warmup, iteration, perf);
  test(1024 * 352, warmup, iteration, perf);
  test(1024 * 384, warmup, iteration, perf);
  test(1024 * 416, warmup, iteration, perf);
  test(1024 * 448, warmup, iteration, perf);
  test(1024 * 480, warmup, iteration, perf);
  test(1024 * 512, warmup, iteration, perf);
  test(1024 * 768, warmup, iteration, perf);
  test(1024 * 1024, warmup, iteration, perf);
  test(1024 * 1024 * 2, warmup, iteration, perf);
  test(1024 * 1024 * 4, warmup, iteration, perf);
  test(1024 * 1024 * 8, warmup, iteration, perf);
  test(1024 * 1024 * 16, warmup, iteration, perf);
  test(1024 * 1024 * 24, warmup, iteration, perf);
  test(1024 * 1024 * 32, warmup, iteration, perf);
  test(1024 * 1024 * 40, warmup, iteration, perf);
  test(1024 * 1024 * 48, warmup, iteration, perf);
  test(1024 * 1024 * 64, warmup, iteration, perf);
  test(1024 * 1024 * 80, warmup, iteration, perf);
  test(1024 * 1024 * 96, warmup, iteration, perf);
  test(1024 * 1024 * 128, warmup, iteration, perf);
  test(1024 * 1024 * 160, warmup, iteration, perf);
  test(1024 * 1024 * 192, warmup, iteration, perf);
  test(1024 * 1024 * 224, warmup, iteration, perf);
  test(1024 * 1024 * 256, warmup, iteration, perf);
  test(1024 * 1024 * 384, warmup, iteration, perf);
  test(1024 * 1024 * 512, warmup, iteration, perf);
  test(1024 * 1024 * 1024, warmup, iteration, perf);
  printf("Results are written to memory_latency.csv\n");
  return 0;
}
