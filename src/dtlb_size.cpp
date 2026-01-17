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
#include <string.h>
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
// use same physical page
bool same_ppn = false;

char **generate_random_pointer_chasing_dtlb(size_t size, char ***entry) {
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
    assert(buffer != MAP_FAILED);

    // map pages individually to avoid huge page merging
    for (uint64_t i = 0; i < index_count; i++) {
      assert(mmap((void *)((size_t)base + page_size * i), size,
                  PROT_READ | PROT_WRITE, flags | MAP_FIXED, -1,
                  0) != MAP_FAILED);
    }
  } else {
    buffer = (char **)mmap(0x0, size, PROT_READ | PROT_WRITE, flags, -1, 0);
  }
  assert(buffer != MAP_FAILED);
  memset(buffer, 0xFF, size);
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
  // place pointers at different cache line indices
  // to avoid l1 dcache miss
  for (uint64_t i = 0; i < index_count - 1; i++) {
    buffer[index[i] * page_pointer_count +
           (i * cache_line_size % page_size) / 8] =
        (char *)&buffer[index[i + 1] * page_pointer_count +
                        ((i + 1) * cache_line_size % page_size) / 8];
  }
  buffer[index[index_count - 1] * page_pointer_count +
         ((index_count - 1) * cache_line_size % page_size) / 8] =
      (char *)&buffer[index[0] * page_pointer_count];

  // entrypoint might not be buffer[0]
  *entry = (char **)&buffer[index[0] * page_pointer_count];

  delete[] index;

  return buffer;
}

char **generate_random_pointer_chasing_dtlb_same_ppn(size_t size,
                                                     char ***entry) {
  if (size < (size_t)page_size || (size % page_size != 0)) {
    return NULL;
  }

  uint64_t page_pointer_count = page_size / sizeof(char *);
  // every page one pointer
  uint64_t index_count = size / page_size;

// to avoid cache size limit, we map many virtual pages to a few physical
// pages. e.g. when page size = 4096, we can store 4096/8=512 pointers in the
// same physical page, we only need to allocate:
// size / page_size / (page_size / sizeof(char *)) physical pages, and map the
// virtual pages to them.
// e.g. virtual page 0-511 maps to physical page 0, virtual page 512-1023 maps
// to physical page 1

// however, in some platforms (e.g. xelite and apple m1 with 4kb page size),
// its vipt cache may have its performance degraded when adjacent pages are
// mapped to the same physical page due to aliasing.
// then we need:
// virtual page 0, 4, 8, ..., 2044 maps to physical page 0
// virtual page 1, 5, 9, ..., 2045 maps to physical page 1
// virtual page 2, 6, 10, ..., 2046 maps to physical page 2
// virtual page 3, 7, 11, ..., 2047 maps to physical page 3
#ifdef QUALCOMM_ORYON
  // for xelite, its shmlba is 16kb instead of page size.
  size_t shmlba = 16384;
#else
  size_t shmlba = getpagesize();
#endif

  // ref:
  // https://stackoverflow.com/questions/7335007/how-to-map-two-virtual-adresses-on-the-same-physical-memory-on-linux

  // step 1: reserve virtual memory range
  char *reserved = (char *)mmap(0x0, size, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(reserved != MAP_FAILED);

  // step 2: allocate physical memory pages
  // round up to multiples of shmlba
  size_t phys_size =
      (index_count * sizeof(char *) + shmlba - 1) / shmlba * shmlba;
#ifndef ANDROID
  int fd = shm_open("/dtlb_size_same_ppn", O_CREAT | O_RDWR, 0666);
#else
  int fd = -1;
#endif
  assert(fd >= 0);
  assert(ftruncate(fd, phys_size) >= 0);

  // map whole range and memset it
  // char *temp = (char *)mmap(reserved, phys_size, PROT_READ | PROT_WRITE,
  //                           MAP_SHARED | MAP_FIXED, fd, 0);
  // memset(temp, 0xFF, size);

  // step 3: remap virtual memory range to shared pages
  for (char *page = reserved; page < reserved + size; page += page_size) {
    size_t offset =
        ((page - reserved) / page_size * sizeof(char *)) / shmlba * shmlba;
    // offset within shmlba
    size_t page_offset = (page - reserved) % shmlba;
    char *temp = (char *)mmap(page, page_size, PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_FIXED, fd, offset + page_offset);
    assert(temp != MAP_FAILED);
    assert(temp == page);
  }
  char **buffer = (char **)reserved;
  memset(buffer, 0xFF, size);

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
  // firestorm & oryon has utag based on va
  // we have multiple va mapped to the same pa but has different utag
  // therefore l1d high miss rate
  auto get_offset = [](int i, int page_size) {
    // if page_size = 4096, then offset is [11:3]
    uint64_t offset = i * sizeof(char *);
    offset %= page_size;
    return offset;
  };
  for (uint64_t i = 0; i < index_count - 1; i++) {
    buffer[index[i] * page_pointer_count + get_offset(i, page_size) / 8] =
        (char *)&buffer[index[i + 1] * page_pointer_count +
                        get_offset(i + 1, page_size) / 8];
  }
  buffer[index[index_count - 1] * page_pointer_count +
         get_offset(index_count - 1, page_size) / 8] =
      (char *)&buffer[index[0] * page_pointer_count];

  // entrypoint might not be buffer[0]
  *entry = (char **)&buffer[index[0] * page_pointer_count];

  delete[] index;

  // debugging
  if (1) {
    printf("Test with size %ld\n", size);
    // validate physical addresses
    std::vector<uintptr_t> all_phys;
    for (char *page = reserved; page < reserved + size; page += page_size) {
      uintptr_t phys;
      virt_to_phys_user(&phys, (uintptr_t)page);
      all_phys.push_back(phys);
    }

    for (size_t i = 0; i < 10 && i < all_phys.size(); i++) {
      printf("Phys: %p\n", (void *)all_phys[i]);
    }

    // print 10 pointers on the chain
    char **p = (char **)entry;
    for (int i = 0; i < 10; i++) {
      uintptr_t phys;
      virt_to_phys_user(&phys, (uintptr_t)p);
      printf("%d: %p -> %p\n", i, p, (void *)phys);
      p = (char **)*p;
    }

    // check chain length
    char **q = p;
    unsigned int length = 0;
    while (true) {
      length++;
      q = (char **)*q;
      if (q == p) {
        break;
      }
    }

    assert(length == index_count);
  }

  return buffer;
}

// learned from lmbench lat_mem_rd
#define ONE p = (char **)*p;

// measure memory latency with pointer chasing
void test(int size, int iterations, bool perf, bool perf_full) {
  char **entry;
  char **buffer = NULL;
  if (same_ppn) {
    buffer =
        generate_random_pointer_chasing_dtlb_same_ppn(size * page_size, &entry);
  } else {
    buffer = generate_random_pointer_chasing_dtlb(size * page_size, &entry);
  }
  if (!buffer) {
    // skipped
    return;
  }

  // benchmark
  int n = 1000;
  int repeat = 100;
  std::vector<uint64_t> elapsed;
  std::vector<uint64_t> cycles_elapsed;
  std::vector<uint64_t> l1d_misses_elapsed;
  std::vector<uint64_t> l1dtlb_misses_elapsed;
  std::vector<uint64_t> l2dtlb_misses_elapsed;
  for (int i = 0; i < n; i++) {
    uint64_t before = get_time();
    uint64_t cycles_before = 0;
    uint64_t l1d_misses_before = 0;
    uint64_t l1dtlb_misses_before = 0;
    uint64_t l2dtlb_misses_before = 0;
    if (perf) {
      cycles_before = perf_read_cycles();
      if (perf_full) {
        l1d_misses_before = perf_read_l1d_misses();
        l1dtlb_misses_before = perf_read_l1dtlb_misses();
        l2dtlb_misses_before = perf_read_l2dtlb_misses();
      }
    }

    char **p = entry;
    for (int i = 0; i < iterations; i++) {
      HUNDRED(ONE);
    }

    if (i > 10) {
      // avoid optimization
      *(volatile char *)*p;
      uint64_t after = get_time();
      elapsed.push_back(after - before);

      uint64_t cycles_after = 0;
      if (perf) {
        cycles_after = perf_read_cycles();
      }
      cycles_elapsed.push_back(cycles_after - cycles_before);

      uint64_t l1d_misses_after = 0;
      if (perf_full) {
        l1d_misses_after = perf_read_l1d_misses();
      }
      l1d_misses_elapsed.push_back(l1d_misses_after - l1d_misses_before);

      uint64_t l1dtlb_misses_after = 0;
      if (perf_full) {
        l1dtlb_misses_after = perf_read_l1dtlb_misses();
      }
      l1dtlb_misses_elapsed.push_back(l1dtlb_misses_after -
                                      l1dtlb_misses_before);

      uint64_t l2dtlb_misses_after = 0;
      if (perf_full) {
        l2dtlb_misses_after = perf_read_l2dtlb_misses();
      }
      l2dtlb_misses_elapsed.push_back(l2dtlb_misses_after -
                                      l2dtlb_misses_before);
    }
  }

  if (perf) {
    if (perf_full) {

      fprintf(fp, "%d,%.2f,%.2f,%.2f,%.2f,%.2f\n", size,
              (double)*std::min_element(elapsed.begin(), elapsed.end()) /
                  iterations / repeat,
              (double)*std::min_element(cycles_elapsed.begin(),
                                        cycles_elapsed.end()) /
                  iterations / repeat,
              (double)*std::min_element(l1d_misses_elapsed.begin(),
                                        l1d_misses_elapsed.end()) /
                  iterations / repeat,
              (double)*std::min_element(l1dtlb_misses_elapsed.begin(),
                                        l1dtlb_misses_elapsed.end()) /
                  iterations / repeat,
              (double)*std::min_element(l2dtlb_misses_elapsed.begin(),
                                        l2dtlb_misses_elapsed.end()) /
                  iterations / repeat);
    } else {
      fprintf(fp, "%d,%.2f,%.2f\n", size,
              (double)*std::min_element(elapsed.begin(), elapsed.end()) /
                  iterations / repeat,
              (double)*std::min_element(cycles_elapsed.begin(),
                                        cycles_elapsed.end()) /
                  iterations / repeat);
    }
  } else {
    fprintf(fp, "%d,%.2f\n", size,
            (double)*std::min_element(elapsed.begin(), elapsed.end()) /
                iterations / repeat);
  }
  fflush(fp);

  munmap(buffer, size * page_size);
}

int main(int argc, char *argv[]) {
  fp = fopen("dtlb_size.csv", "w");
  assert(fp);
  bind_to_core();

  int iteration = 1000;
  bool perf = false;
  bool perf_full = false;
  int from_pages = 1;
  int to_pages = 1024;

  page_size = getpagesize();
  int opt;
  while ((opt = getopt(argc, argv, "P:i:pFHhsf:t:")) != -1) {
    switch (opt) {
    case 'H':
      huge_tlb = true;
      break;
    case 's':
      same_ppn = true;
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
    case 'F':
      perf = true;
      perf_full = true;
      break;
    case 'f':
      sscanf(optarg, "%d", &from_pages);
      break;
    case 't':
      sscanf(optarg, "%d", &to_pages);
      break;
    default:
      fprintf(stderr,
              "Usage: %s [-P page_size] [-i iteration] [-f from_pages] [-t "
              "to_pages] [-p] [-F] [-H] [-h] [-s]\n",
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

  if (same_ppn) {
    printf("Map pages to the same physical pages\n");
  }

  if (perf) {
    setup_perf_cycles();
    if (perf_full) {
      setup_perf_l1d_misses();
      setup_perf_l1dtlb_misses();
      setup_perf_l2dtlb_misses();
    }
  }

  if (perf) {
    if (perf_full) {
      fprintf(fp,
              "pages,time(ns),cycles,l1d_misses,l1dtlb_misses,l2dtlb_misses\n");
    } else {
      fprintf(fp, "pages,time(ns),cycles\n");
    }
  } else {
    fprintf(fp, "pages,time(ns)\n");
  }
  for (int i = from_pages; i <= to_pages; i++) {
    test(i, iteration, perf, perf_full);
  }
  printf("Results are written to dtlb_size.csv\n");
#ifndef ANDROID
  shm_unlink("/dtlb_size_same_ppn");
#endif
  return 0;
}
