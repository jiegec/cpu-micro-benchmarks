#ifndef __UTILS_H__
#define __UTILS_H__

#include <assert.h>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// detect host machine if not set
#if !defined(HOST_AARCH64) && !defined(HOST_AMD64) && !defined(HOST_LOONGARCH64)
#ifdef __x86_64__
#define HOST_AMD64
#endif
#ifdef __aarch64__
#define HOST_AARCH64
#endif
#ifdef __loongarch__
#define HOST_LOONGARCH64
#endif
#endif

// learned from lmbench lat_mem_rd
#define FIVE(X) X X X X X
#define TEN(X) FIVE(X) FIVE(X)
#define FIFTY(X) TEN(X) TEN(X) TEN(X) TEN(X) TEN(X)
#define HUNDRED(X) FIFTY(X) FIFTY(X)
#define THOUSAND(X) HUNDRED(TEN(X))

// utilities
std::map<const char *, size_t> get_cache_sizes();
char **generate_random_pointer_chasing(size_t size);

// get time or cycles
// unit: ns or cycle
uint64_t get_time();
// prioritize cycle over time
void setup_time_or_cycles();
uint64_t get_time_or_cycles();

void bind_to_core();

// pmu related
struct counter_per_cycle {
  uint64_t counter;
  uint64_t cycles;
};

// top down perf analysis
struct top_down {
  // level 1
  double frontend_bound;
  double bad_speculation;
  double retiring;
  double backend_bound;

  top_down &operator+=(const top_down &other) {
    this->frontend_bound += other.frontend_bound;
    this->bad_speculation += other.bad_speculation;
    this->retiring += other.retiring;
    this->backend_bound += other.backend_bound;
    return *this;
  }
};

// mispredictions per kilo instructions
struct mpki {
  uint64_t mispredictions;
  uint64_t instructions;
  double mpki;
};

// declare pmu counters
#define DECLARE_COMPUTED_COUNTER(type, name)                                   \
  void perf_begin_##name();                                                    \
  type perf_end_##name();                                                      \
  void setup_perf_##name();

#define DECLARE_RAW_COUNTER(name)                                              \
  uint64_t perf_read_##name();                                                 \
  void setup_perf_##name();

#include "include/counters.h"
#undef DECLARE_COMPUTED_COUNTER
#undef DECLARE_RAW_COUNTER

struct perf_event_mmap_page;
struct raw_perf_counter {
  // perf fd
  int fd;
  // mmapped page for userspace access
  struct perf_event_mmap_page *page;
  // group id
  uint64_t id;

  raw_perf_counter();
  uint64_t read() const;
  uint64_t read_syscall() const;
  uint64_t read_userspace() const;
  void reset() const;
};
raw_perf_counter setup_perf_common_failable(uint32_t type, uint64_t config);
// can never fail, exit on failure
raw_perf_counter setup_perf_common(uint32_t type, uint64_t config);

// generate assemblies
extern bool nasm;
void define_gadgets_array(FILE *fp, const char *name);
void add_gadget(FILE *fp, const char *format, ...);
void emit_nasm_nops(FILE *fp, int repeat);
// load address to register on arm64
void arm64_la(FILE *fp, int reg, const char *format, ...);

// convert virtual address to physical
int virt_to_phys_user(uintptr_t *paddr, uintptr_t vaddr);

#endif
