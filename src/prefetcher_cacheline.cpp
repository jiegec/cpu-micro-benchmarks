#include "include/utils.h"
#include <cstdint>
#include <getopt.h>
#include <set>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#ifdef HOST_AMD64
#include <emmintrin.h>
#include <x86gprintrin.h>
#define FENCE() _mm_mfence()
#define FLUSH(addr) _mm_clflush(addr)
#define CACHELINE_SIZE 64
#elif defined(HOST_PPC64LE)
static inline void ppc_sync(void) { asm volatile("sync" ::: "memory"); }
static inline void ppc_dcbf(void *addr) {
  asm volatile("dcbf 0, %0" ::"r"(addr) : "memory");
}
#define FENCE() ppc_sync()
#define FLUSH(addr) ppc_dcbf(addr)
#define CACHELINE_SIZE 128
#else
#define FENCE()
#define FLUSH(addr)
#define CACHELINE_SIZE 64
#endif

// https://abertschi.ch/blog/2022/prefetching/

static inline uint64_t measure_access(char *addr) {
  uint64_t t00 = 0;
  uint64_t t01 = 0;

  FENCE();
  t00 = perf_read_cycles();
  FENCE();

  *(volatile char *)addr;

  FENCE();
  t01 = perf_read_cycles();
  FENCE();

  uint64_t cycles = (int)(t01 - t00);
  return cycles;
}

int main(int argc, char *argv[]) {
  FILE *fp;
  fp = fopen("prefetcher_cacheline.csv", "w");
  assert(fp);
  bind_to_core();
  setup_perf_cycles();

  /*
   * Sample code snippet to illustrate the gist of the timing measurements.
   * Here we measure prefetching impact when accessing TARGET,
   * a single cache line.
   */
  const int N = 1000;
  const int M = 80;
  const long CL = CACHELINE_SIZE;

  int opt;
  int from = 0;
  int count = 5;
  int stride = 5;
  while ((opt = getopt(argc, argv, "f:c:s:")) != -1) {
    switch (opt) {
    case 'f':
      sscanf(optarg, "%d", &from);
      break;
    case 'c':
      sscanf(optarg, "%d", &count);
      break;
    case 's':
      sscanf(optarg, "%d", &stride);
      break;
    default:
      fprintf(stderr, "Usage: %s [-f from] [-c count] [-s stride]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  std::set<int> accesses;
  for (int i = 0; i < count; i++) {
    accesses.insert(from + i * stride);
  }

  char *probe_array = (char *)mmap(NULL, M * CL, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  // force allocation
  for (int i = 0; i < M * CL; i += 1) {
    probe_array[i] = i;
  }

  // check if phys continuous
  bool is_continuous = true;
  bool unknown = false;
  uintptr_t prev_paddr;
  for (int i = 0; i < M; i += getpagesize() / CL) {
    char *base = &probe_array[i * CL];
    uintptr_t paddr;
    if (virt_to_phys_user(&paddr, (uintptr_t)base) != 0 || paddr == 0) {
      // failed
      unknown = true;
      break;
    }
    printf("%p -> %p\n", base, (void *)paddr);
    if (i != 0) {
      if (paddr - prev_paddr != (uintptr_t)getpagesize()) {
        is_continuous = false;
      }
    }
    prev_paddr = paddr;
  }
  if (unknown) {
    printf("Failed to check if physical pages are continuous\n");
  } else {
    printf("Physical pages %s continuous\n", is_continuous ? "are" : "are not");
  }

  std::vector<uint64_t> access_times[M];

  // # of measurements
  for (int n = 0; n < N; n++) {

    // only probe one cacheline per experiment
    for (int el = 0; el < M; el++) {

      // flush the probing array
      for (int i = 0; i < M; i++) {
        FLUSH(probe_array + i * CL);
      }
      FENCE();

      // Desired access pattern
      for (int cl : accesses) {
        *(volatile char *)(probe_array + cl * CL);
      }
      FENCE();

      // check which elements have been prefetched
      access_times[el].push_back(measure_access(probe_array + el * CL));
    }
  }

  fprintf(fp, "cacheline,time,is_access,min,max\n");
  for (int i = 0; i < M; i++) {
    double sum = 0;
    double min = access_times[i][0];
    double max = access_times[i][0];
    for (size_t j = 0; j < access_times[i].size(); j++) {
      sum += access_times[i][j];
      if (min > access_times[i][j]) {
        min = access_times[i][j];
      }
      if (max < access_times[i][j]) {
        max = access_times[i][j];
      }
    }
    fprintf(fp, "%d,%.2lf,%d,%.0lf,%.0lf\n", i, sum / N,
            accesses.find(i) != accesses.end(), min, max);
  }
}
