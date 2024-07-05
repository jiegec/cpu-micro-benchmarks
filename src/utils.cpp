#include "include/utils.h"
#include <assert.h>
#include <memory.h>
#include <random>
#include <sys/time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#ifdef __linux__
#include <linux/perf_event.h>
#include <sched.h>
#include <sys/syscall.h>
#endif

#ifdef __x86_64__
#include <x86intrin.h>
#endif

// https://clang.llvm.org/docs/LanguageExtensions.html#has-builtin
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

std::map<const char *, size_t> get_cache_sizes() {
  std::map<const char *, size_t> result;
#ifdef __linux__
  int keys[] = {_SC_LEVEL1_ICACHE_SIZE, _SC_LEVEL1_DCACHE_SIZE,
                _SC_LEVEL2_CACHE_SIZE, _SC_LEVEL3_CACHE_SIZE};
  const char *names[] = {
      "L1i",
      "L1d",
      "L2",
      "L3",
  };
  for (int i = 0; i < 3; i++) {
    long size = sysconf(keys[i]);
    if (size != -1) {
      result[names[i]] = size;
    }
  }
#elif defined(__APPLE__)
  const char *keys[] = {"hw.l1dcachesize", "hw.l1icachesize", "hw.l2cachesize"};
  const char *names[] = {
      "L1d",
      "L1i",
      "L2",
  };
  for (int i = 0; i < 3; i++) {
    size_t size = 0;
    size_t len = sizeof(size);
    int ret = sysctlbyname(keys[i], &size, &len, nullptr, 0);
    if (ret != -1) {
      result[names[i]] = size;
    }
  }
#endif
  return result;
}

uint64_t get_time() {
  struct timeval tv = {};
  gettimeofday(&tv, nullptr);
  return (uint64_t)tv.tv_sec * 1000000000 + (uint64_t)tv.tv_usec * 1000;
}

char **generate_random_pointer_chasing(size_t size) {
  int page_size = getpagesize();
  if (size < page_size) {
    return NULL;
  }

  int page_pointer_count = page_size / sizeof(char *);
  int count = size / sizeof(char *);
  // every page one pointer
  int index_count = size / page_size;
  char **buffer = new char *[count];
  int *index = new int[index_count];

  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());

  // init index and shuffle
  for (int i = 0; i < index_count; i++) {
    index[i] = i;
  }
  for (int i = 1; i < index_count; i++) {
    std::uniform_int_distribution<int> distr(0, i - 1);
    int j = distr(generator);
    int temp = index[i];
    index[i] = index[j];
    index[j] = temp;
  }

  // init circular list
  for (int i = 0; i < index_count - 1; i++) {
    buffer[index[i] * page_pointer_count] =
        (char *)&buffer[index[i + 1] * page_pointer_count];
  }
  buffer[index[index_count - 1] * page_pointer_count] =
      (char *)&buffer[index[0] * page_pointer_count];

  delete[] index;

  return buffer;
}

#ifdef __linux__
uint64_t perf_read_common(int fd) {
  uint64_t counter;
  int res = read(fd, &counter, sizeof(counter));
  assert(res == sizeof(counter));
  return counter;
}

int setup_perf_common(uint32_t type, uint64_t config) {
  struct perf_event_attr *attr =
      (struct perf_event_attr *)malloc(sizeof(struct perf_event_attr));
  memset(attr, 0, sizeof(struct perf_event_attr));
  attr->type = type;
  attr->size = sizeof(struct perf_event_attr);
  attr->config = config;
  attr->disabled = 0;
  attr->pinned = 1;
  attr->inherit = 1;
  attr->exclude_kernel = 1;
  int fd = syscall(SYS_perf_event_open, attr, 0, -1, -1, 0);
  if (fd < 0) {
    perror("perf_event_open");
    fprintf(stderr, "try: sudo sysctl kernel.perf_event_paranoid=2");
    exit(1);
  }
  return fd;
}

int perf_fd_cycles = -1;

uint64_t perf_read_cycles() { return perf_read_common(perf_fd_cycles); }

void setup_perf_cycles() {
  perf_fd_cycles =
      setup_perf_common(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
  if (perf_fd_cycles >= 0) {
    printf("Using PMU to count cycles\n");
  }
}

int perf_fd_instructions = -1;

uint64_t perf_read_instructions() {
  return perf_read_common(perf_fd_instructions);
}

void setup_perf_instructions() {
  perf_fd_instructions =
      setup_perf_common(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
}

int perf_fd_branch_misses = -1;

uint64_t perf_read_branch_misses() {
  return perf_read_common(perf_fd_branch_misses);
}

void setup_perf_branch_misses() {
  perf_fd_branch_misses =
      setup_perf_common(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
}

int perf_fd_llc_misses = -1;
int perf_fd_llc_loads = -1;

uint64_t perf_read_llc_misses() { return perf_read_common(perf_fd_llc_misses); }

uint64_t perf_read_llc_loads() { return perf_read_common(perf_fd_llc_loads); }

void setup_perf_llc_misses() {
  perf_fd_llc_misses = setup_perf_common(
      PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) |
                              (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                              (PERF_COUNT_HW_CACHE_RESULT_MISS << 16));
}

void setup_perf_llc_loads() {
  perf_fd_llc_loads = setup_perf_common(
      PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) |
                              (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                              (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16));
}
#endif

void setup_time_or_cycles() {
#ifdef __linux__
  setup_perf_cycles();
#endif
}

uint64_t get_time_or_cycles() {
#if defined(__linux__)
  if (perf_fd_cycles >= 0) {
    // cycle
    return perf_read_cycles();
  } else {
    // time
    return get_time();
  }
#else
  // time
  return get_time();
#endif
}

// bind to core 0
void bind_to_core() {
#ifdef __linux__
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(0, &set);
  int res = sched_setaffinity(0, sizeof(set), &set);
  if (res == 0) {
    printf("Pinned to cpu 0\n");
  }
#endif
}
