// reference:
// https://github.com/intel/lmbench/blob/master/src/lat_mem_rd.c

#include "include/utils.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

// learned from lmbench lat_mem_rd
#define ONE p = (char **)*p;

FILE *fp;

// measure memory latency with pointer chasing
void test(int size, int warmup, int iterations, bool perf, bool perf_full) {
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
  uint64_t cycles_before = 0, l2c_misses_before = 0, l2c_accesses_before = 0;
  if (perf) {
    cycles_before = perf_read_cycles();
    if (perf_full) {
      l2c_misses_before = perf_read_l2c_misses();
      l2c_accesses_before = perf_read_l2c_accesses();
    }
  }

  for (int i = 0; i < iterations; i++) {
    HUNDRED(ONE);
  }

  // avoid optimization
  *(volatile char *)*p;
  uint64_t after = get_time();
  uint64_t cycles_after = 0, l2c_misses_after = 0, l2c_accesses_after = 0;
  if (perf) {
    cycles_after = perf_read_cycles();
    if (perf_full) {
      l2c_misses_after = perf_read_l2c_misses();
      l2c_accesses_after = perf_read_l2c_accesses();
    }
  }

  if (perf_full) {
    fprintf(fp, "%d,%.2f,%.2f,%.2f,%.2f\n", size,
            (double)(after - before) / iterations / 100,
            (double)(cycles_after - cycles_before) / iterations / 100,
            (double)(l2c_misses_after - l2c_misses_before) / iterations / 100,
            (double)(l2c_accesses_after - l2c_accesses_before) / iterations /
                100);
  } else if (perf) {
    fprintf(fp, "%d,%.2f,%.2f\n", size,
            (double)(after - before) / iterations / 100,
            (double)(cycles_after - cycles_before) / iterations / 100);
  } else {
    fprintf(fp, "%d,%.2f\n", size, (double)(after - before) / iterations / 100);
  }
  fflush(fp);

  delete[] buffer;
}

int main(int argc, char *argv[]) {
  fp = fopen("memory_latency.csv", "w");
  assert(fp);
  bind_to_core();

  int warmup = 50000;
  int iteration = 500000;
  bool perf = false;
  bool perf_full = false;

  int opt;
  while ((opt = getopt(argc, argv, "w:i:pf")) != -1) {
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
    case 'f':
      perf = true;
      perf_full = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-w warmup] [-i iteration] [-p] [-f]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (perf) {
    setup_perf_cycles();
    if (perf_full) {
      setup_perf_l2c_misses();
      setup_perf_l2c_accesses();
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

  if (perf_full) {
    fprintf(fp, "size,time(ns),cycles,l2c miss ratio,l2c load ratio\n");
  } else if (perf) {
    fprintf(fp, "size,time(ns),cycles\n");
  } else {
    fprintf(fp, "size,time(ns)\n");
  }
  test(1024, warmup, iteration, perf, perf_full);
  test(1024 * 2, warmup, iteration, perf, perf_full);
  test(1024 * 4, warmup, iteration, perf, perf_full);
  test(1024 * 16, warmup, iteration, perf, perf_full);
  test(1024 * 24, warmup, iteration, perf, perf_full);
  test(1024 * 32, warmup, iteration, perf, perf_full);
  test(1024 * 28, warmup, iteration, perf, perf_full);
  test(1024 * 36, warmup, iteration, perf, perf_full);
  test(1024 * 40, warmup, iteration, perf, perf_full);
  test(1024 * 44, warmup, iteration, perf, perf_full);
  test(1024 * 48, warmup, iteration, perf, perf_full);
  test(1024 * 64, warmup, iteration, perf, perf_full);
  test(1024 * 80, warmup, iteration, perf, perf_full);
  test(1024 * 96, warmup, iteration, perf, perf_full);
  test(1024 * 112, warmup, iteration, perf, perf_full);
  test(1024 * 128, warmup, iteration, perf, perf_full);
  test(1024 * 144, warmup, iteration, perf, perf_full);
  test(1024 * 160, warmup, iteration, perf, perf_full);
  test(1024 * 176, warmup, iteration, perf, perf_full);
  test(1024 * 192, warmup, iteration, perf, perf_full);
  test(1024 * 208, warmup, iteration, perf, perf_full);
  test(1024 * 216, warmup, iteration, perf, perf_full);
  test(1024 * 224, warmup, iteration, perf, perf_full);
  test(1024 * 232, warmup, iteration, perf, perf_full);
  test(1024 * 240, warmup, iteration, perf, perf_full);
  test(1024 * 256, warmup, iteration, perf, perf_full);
  test(1024 * 288, warmup, iteration, perf, perf_full);
  test(1024 * 320, warmup, iteration, perf, perf_full);
  test(1024 * 352, warmup, iteration, perf, perf_full);
  test(1024 * 384, warmup, iteration, perf, perf_full);
  test(1024 * 416, warmup, iteration, perf, perf_full);
  test(1024 * 448, warmup, iteration, perf, perf_full);
  test(1024 * 480, warmup, iteration, perf, perf_full);
  test(1024 * 512, warmup, iteration, perf, perf_full);
  test(1024 * 768, warmup, iteration, perf, perf_full);
  test(1024 * 1024, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 1.125, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 1.25, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 1.5, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 2, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 3, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 4, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 5, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 6, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 7, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 8, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 9, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 10, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 11, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 12, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 13, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 14, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 16, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 24, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 32, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 40, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 48, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 64, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 80, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 96, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 128, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 160, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 192, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 224, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 256, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 384, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 512, warmup, iteration, perf, perf_full);
  test(1024 * 1024 * 1024, warmup, iteration, perf, perf_full);
  printf("Results are written to memory_latency.csv\n");
  return 0;
}
