#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// ref:
// Rage Against the Machine Clear: A Systematic Analysis of Machine Clears and
// Their Implications for Transient Execution Attacks
// use with generate_gadget tool

// defined in gen_rob_test()
// args: ptr1
typedef char **(*gadget)(char *);
extern "C" {
extern gadget memory_dependency_predictor_reverse_global_gadgets[];
}

void memory_dependency_predictor_reverse_global(FILE *fp) {
  bind_to_core();
  setup_perf_machine_clear();
  setup_perf_machine_clear();
  int repeat = 1000;
  int num_sizes = 100;

  char buffer[256] __attribute__((aligned(64)));
  fprintf(fp, "size,min,avg,max\n");
  for (int size = 1; size < num_sizes; size++) {
    std::vector<int> history;
    for (int iteration = 0; iteration < repeat; iteration++) {
      // warmup
      memory_dependency_predictor_reverse_global_gadgets[0](buffer);
      uint64_t begin = perf_read_machine_clear();
      memory_dependency_predictor_reverse_global_gadgets[size](buffer);
      uint64_t elapsed = perf_read_machine_clear() - begin;

      // cpi
      if (elapsed <= 100) // drop outliers
        history.push_back((double)elapsed);
    }

    double min = history[0];
    double max = history[0];
    double sum = 0;
    for (size_t i = 0; i < history.size(); i++) {
      sum += history[i];
      if (min > history[i]) {
        min = history[i];
      }
      if (max < history[i]) {
        max = history[i];
      }
    }

    fprintf(fp, "%d,%.2lf,%.2lf,%.2lf\n", size, min, sum / repeat, max);
    fflush(fp);
  }
}
