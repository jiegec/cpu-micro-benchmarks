#include "include/utils.h"
#include <unistd.h>

int res = 0;

void test_1() {
  int sum = 0;
  int n = 1000;
  for (int i = 0; i < n; i++) {
    if (i * i < n) {
      sum += i * i;
    } else {
      sum += i * i * i;
    }
  }
  res += sum;
}

int main(int argc, char *argv[]) {

  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
    switch (opt) {
    default:
      fprintf(stderr, "Usage: %s [-p]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  bind_to_core();
  setup_perf_instructions();
  setup_perf_cycles();

  int warmup = 1000;

  for (int i = 0; i < warmup; i++) {
    test_1();
  }

  int m = 100000;
  uint64_t cycles_before = perf_read_cycles();
  uint64_t instructions_before = perf_read_instructions();

  for (int i = 0; i < m; i++) {
    test_1();
  }

  uint64_t cycles_after = perf_read_cycles();
  uint64_t instructions_after = perf_read_instructions();

  printf("%ld cycles, %ld instructions, %.2lf ipc, %d ans\n",
         (cycles_after - cycles_before) / m,
         (instructions_after - instructions_before) / m,
         (double)(instructions_after - instructions_before) /
             (cycles_after - cycles_before),
         res);
  return 0;
}
