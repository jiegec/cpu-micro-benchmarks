#include "include/utils.h"
#include <stdio.h>
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

int main() {
  bind_to_core();
  setup_time_or_cycles();
  uint64_t begin = get_time_or_cycles();
  test_int_add();
  uint64_t unit_elapsed = get_time_or_cycles() - begin;
  for (auto it : tests) {
    begin = get_time_or_cycles();
    it.second();
    uint64_t elapsed = get_time_or_cycles() - begin;
    printf("%s: %.2lf cycles\n", it.first, (double)elapsed / unit_elapsed);
  }
}