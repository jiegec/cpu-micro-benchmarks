#include "include/utils.h"
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <random>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#ifdef __linux__
#include <linux/mman.h>
#endif

FILE *fp;
int page_size = -1;
// use huge tlb
bool huge_tlb = false;
// avoid huge page merging
bool avoid_hugepage_merging = false;

char **generate_random_pointer_chasing_dtlb(size_t size) {
  if (size < (size_t)page_size || (size % page_size != 0)) {
    return NULL;
  }

  uint64_t cache_line_size = 64;
  uint64_t page_pointer_count = page_size / sizeof(char *);
  // every page one pointer
  uint64_t index_count = size / page_size;
  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_HUGETLB
  flags |= huge_tlb ? (MAP_HUGETLB | MAP_HUGE_2MB) : 0;
#endif
  char **buffer = (char **)MAP_FAILED;
  if (avoid_hugepage_merging) {
    void *base = (void *)0x100000000;
    buffer = (char **)mmap(base, size, PROT_READ | PROT_WRITE,
                           flags | MAP_FIXED, -1, 0);
    for (uint64_t i = 0; i < index_count; i++) {
      assert(mmap((void *)((size_t)base + page_size * i), size,
                  PROT_READ | PROT_WRITE, flags | MAP_FIXED, -1,
                  0) != MAP_FAILED);
    }
  } else {
    buffer = (char **)mmap(0x0, size, PROT_READ | PROT_WRITE, flags, -1, 0);
  }
  assert(buffer != MAP_FAILED);
  uint64_t *index = new uint64_t[index_count];

  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());

  // init index and shuffle
  for (uint64_t i = 0; i < index_count; i++) {
    index[i] = i;
  }
  for (uint64_t i = 1; i < index_count; i++) {
    std::uniform_int_distribution<int> distr(0, i - 1);
    int j = distr(generator);
    uint64_t temp = index[i];
    index[i] = index[j];
    index[j] = temp;
  }

  // init circular list
  for (uint64_t i = 0; i < index_count - 1; i++) {
    buffer[index[i] * page_pointer_count +
           (i * cache_line_size % page_size) / 8] =
        (char *)&buffer[index[i + 1] * page_pointer_count +
                        ((i + 1) * cache_line_size % page_size) / 8];
  }
  buffer[index[index_count - 1] * page_pointer_count +
         ((index_count - 1) * cache_line_size % page_size) / 8] =
      (char *)&buffer[index[0] * page_pointer_count];

  // handle if index[0] != 0
  if (index[0] != 0)
    buffer[0] = (char *)&buffer[index[0] * page_pointer_count];

  delete[] index;

  return buffer;
}

// learned from lmbench lat_mem_rd
#define ONE p = (char **)*p;

// measure memory latency with pointer chasing
void test(int size, int iterations, bool perf) {
  char **buffer = generate_random_pointer_chasing_dtlb(size * page_size);
  if (!buffer) {
    // skipped
    return;
  }

  // benchmark
  int n = 1000;
  int repeat = 100;
  std::vector<uint64_t> elapsed;
#ifdef __linux__
  std::vector<uint64_t> cycles_elapsed;
#endif
  for (int i = 0; i < n; i++) {
    uint64_t before = get_time();
#ifdef __linux__
    uint64_t cycles_before = 0;
    if (perf) {
      cycles_before = perf_read_cycles();
    }
#endif

    char **p = (char **)buffer[0];
    for (int i = 0; i < iterations; i++) {
      HUNDRED(ONE);
    }

    if (i > 10) {
      // avoid optimization
      *(volatile char *)*p;
      uint64_t after = get_time();
      elapsed.push_back(after - before);
#ifdef __linux__
      uint64_t cycles_after = 0;
      if (perf) {
        cycles_after = perf_read_cycles();
      }
      cycles_elapsed.push_back(cycles_after - cycles_before);
#endif
    }
  }

#ifdef __linux__
  if (perf) {
    fprintf(fp, "%d,%.2f,%.2f\n", size,
            (double)*std::min_element(elapsed.begin(), elapsed.end()) /
                iterations / repeat,
            (double)*std::min_element(cycles_elapsed.begin(),
                                      cycles_elapsed.end()) /
                iterations / repeat);
  } else {
    fprintf(fp, "%d,%.2f\n", size,
            (double)*std::min_element(elapsed.begin(), elapsed.end()) /
                iterations / repeat);
  }
#else
  fprintf(fp, "%d,%.2f\n", size,
          (double)*std::min_element(elapsed.begin(), elapsed.end()) /
              iterations / repeat);
#endif
  fflush(fp);

  munmap(buffer, size * page_size);
}

int main(int argc, char *argv[]) {
  fp = fopen("dtlb_size.csv", "w");
  assert(fp);
  bind_to_core();

  int iteration = 1000;
  bool perf = false;

  page_size = getpagesize();
  int opt;
  while ((opt = getopt(argc, argv, "P:i:pHh")) != -1) {
    switch (opt) {
    case 'H':
      huge_tlb = true;
      break;
    case 'h':
      avoid_hugepage_merging = true;
      break;
    case 'i':
      sscanf(optarg, "%d", &iteration);
      break;
    case 'P':
      sscanf(optarg, "%d", &page_size);
      break;
    case 'p':
      perf = true;
      break;
    default:
      fprintf(stderr,
              "Usage: %s [-P page_size] [-i iteration] [-p] [-H] [-h]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (huge_tlb) {
    printf("Use hugetlbfs\n");
  } else if (avoid_hugepage_merging) {
    printf("Avoid huge page merging\n");
    printf("Please disable THP via: echo never "
           ">/sys/kernel/mm/transparent_hugepage/enabled\n");
  }

#ifdef __linux__
  if (perf) {
    setup_perf_cycles();
  }
#endif

#ifdef __linux__
  if (perf) {
    fprintf(fp, "pages,time(ns),cycles\n");
  } else {
    fprintf(fp, "pages,time(ns)\n");
  }
#else
  fprintf(fp, "pages,time(ns)\n");
#endif
  for (int i = 1; i < 1024; i++) {
    test(i, iteration, perf);
  }
  printf("Results are written to dtlb_size.csv\n");
  return 0;
}
