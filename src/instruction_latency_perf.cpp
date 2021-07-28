#include "include/utils.h"
#include <assert.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <utility>
#include <vector>

const int N = 1000000;

#define INSTR_TEST(NAME, INST, ...)                                            \
  void test_##NAME() {                                                         \
    for (int i = 0; i < N; i++) {                                              \
      asm volatile(HUNDRED(INST) : : : __VA_ARGS__);                           \
    }                                                                          \
  }

#include "include/instrs.h"

#undef INSTR_TEST

#define INSTR_TEST(NAME, INST, ...) std::make_pair(#NAME, test_##NAME),

std::vector<std::pair<const char *, void (*)()>> tests = {
#include "include/instrs.h"
};

#undef INSTR_TEST

int perf_fd = -1;

uint64_t perf_read() {
  uint64_t counter;
  int res = read(perf_fd, &counter, sizeof(counter));
  assert(res == sizeof(counter));
  return counter;
}

int main() {
  struct perf_event_attr *attr =
      (struct perf_event_attr *)malloc(sizeof(struct perf_event_attr));
  memset(attr, 0, sizeof(attr));
  attr->type = PERF_TYPE_RAW;
  attr->config = 0x3C; // cycles
  attr->disabled = 0;
  attr->pinned = 1;
  attr->inherit = 1;
  attr->exclude_kernel = 1;
  perf_fd = syscall(SYS_perf_event_open, attr, 0, -1, -1, 0);
  if (perf_fd < 0) {
    perror("perf_event_open");
    return 1;
  }

  for (auto it : tests) {
    uint64_t begin = perf_read();
    it.second();
    uint64_t cycles = perf_read() - begin;
    printf("%s: %.2lf cycles\n", it.first, (double)cycles / N / 100);
  }
}