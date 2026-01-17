#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// use with generate_gadget tool

// defined in gen_phr_test()
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget phr_size_gadgets[];
}

void phr_size(FILE *fp) {
  int loop_count = 1000;
  // match gen_phr_test
  int min_size = 1;
  int max_size = 256;

  bind_to_core();
  setup_perf_cycles();
#if defined(NO_COND_BRANCH_MISSES)
  // fallback
  setup_perf_branch_misses();
#else
  setup_perf_cond_branch_misses();
#endif
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "size,min,avg,max,cycles\n");
  int gadget_index = 0;
  for (int size = min_size; size <= max_size; size++) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

    double sum = 0;
    double sum_cycles = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {

      // random
      for (int i = 0; i <= loop_count; i++) {
        buffer[i] = rand() % 2;
      }
      // ensures that ldr w11, [x0, w11, uxtw #2] does not change w11
      buffer[0] = 0;
      buffer[1] = 1;

#if defined(NO_COND_BRANCH_MISSES)
      // fallback
      uint64_t begin = perf_read_branch_misses();
#else
      uint64_t begin = perf_read_cond_branch_misses();
#endif
      uint64_t begin_cycles = perf_read_cycles();

      phr_size_gadgets[gadget_index](loop_count, buffer);

#if defined(NO_COND_BRANCH_MISSES)
      // fallback
      uint64_t elapsed = perf_read_branch_misses() - begin;
#else
      uint64_t elapsed = perf_read_cond_branch_misses() - begin;
#endif
      uint64_t elapsed_cycles = perf_read_cycles() - begin_cycles;

      // skip warmup
      if (i >= 10) {
        double time = (double)elapsed / loop_count;
        history.push_back(time);
        sum += time;

        double cycles = (double)elapsed_cycles / loop_count;
        sum_cycles += cycles;
      }
    }
    gadget_index++;

    double min = history[0];
    double max = history[0];
    for (size_t i = 0; i < history.size(); i++) {
      if (min > history[i]) {
        min = history[i];
      }
      if (max < history[i]) {
        max = history[i];
      }
    }
    fprintf(fp, "%d,%.2lf,%.2lf,%.2lf,%.2lf\n", size, min, sum / history.size(),
            max, sum_cycles / history.size());
    fflush(fp);
  }
  delete[] buffer;
}
