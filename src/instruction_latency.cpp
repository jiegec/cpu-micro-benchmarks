#include "include/utils.h"
#include <stdio.h>
#include <unistd.h>
#include <utility>
#include <vector>

int N = 1000000;

#define INSTR_TEST(NAME, INST, ...)                                            \
  void test_##NAME(int n) {                                                    \
    for (int i = 0; i < n; i++) {                                              \
      asm volatile(HUNDRED(INST) : : : __VA_ARGS__);                           \
    }                                                                          \
  }

#include "include/instrs.h"

#undef INSTR_TEST

#define INSTR_TEST(NAME, INST, ...) std::make_pair(#NAME, test_##NAME),

std::vector<std::pair<const char *, void (*)(int)>> tests = {
#include "include/instrs.h"
};

#undef INSTR_TEST

int main(int argc, char *argv[]) {
  bool perf = false;

  int opt;
  while ((opt = getopt(argc, argv, "n:p")) != -1) {
    switch (opt) {
    case 'n':
      sscanf(optarg, "%d", &N);
      break;
    case 'p':
      perf = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-p]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  bind_to_core();
  if (perf) {
    setup_time_or_cycles();
  }
  uint64_t begin = get_time_or_cycles();
  test_int_add(N);
  uint64_t unit_elapsed = get_time_or_cycles() - begin;
  for (auto it : tests) {
    begin = get_time_or_cycles();
    it.second(N);
    uint64_t elapsed = get_time_or_cycles() - begin;
    std::string name = it.first;
    double cycles = (double)elapsed / unit_elapsed;
    if (name.find("_tp") != std::string::npos) {
      printf("%s: throughput 1/%.2lf=%.2lf instructions\n", it.first, cycles, 1.0 / cycles);
    } else {
      printf("%s: latency %.2lf cycles\n", it.first, cycles);
    }
  }
}
