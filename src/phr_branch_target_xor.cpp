#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// use with generate_gadget tool

// defined in gen_phr_branch_target_xor_test()
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget phr_branch_target_xor_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  // match gen_phr_branch_target_xor_test
#if defined(HOST_AMD64)
  int min_branch_toggle = 1;
  int max_branch_toggle = 15;
  int min_target_toggle = 0;
  int max_target_toggle = 12;
#else
  int min_branch_toggle = 2;
  int max_branch_toggle = 18;
  int min_target_toggle = 2;
  int max_target_toggle = 18;
#endif

  bind_to_core();
#ifdef NO_COND_BRANCH_MISSES
  setup_perf_branch_misses();
#else
  setup_perf_cond_branch_misses();
#endif
  FILE *fp = fopen("phr_branch_target_xor.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "branch,target,min,avg,max\n");
  int gadget_index = 0;
  int repeat = 2; // two branches
  for (int branch_toggle = min_branch_toggle;
       branch_toggle <= max_branch_toggle; branch_toggle++) {
    for (int target_toggle = min_target_toggle;
         target_toggle <= max_target_toggle; target_toggle++) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        for (int i = 0; i <= loop_count; i++) {
          buffer[i] = rand() % 2;
        }
#ifdef NO_COND_BRANCH_MISSES
        uint64_t begin = perf_read_branch_misses();
#else
        uint64_t begin = perf_read_cond_branch_misses();
#endif
        phr_branch_target_xor_gadgets[gadget_index](loop_count, buffer);
#ifdef NO_COND_BRANCH_MISSES
        uint64_t elapsed = perf_read_branch_misses() - begin;
#else
        uint64_t elapsed = perf_read_cond_branch_misses() - begin;
#endif

        // skip warmup
        if (i >= 10) {
          double time = (double)elapsed / loop_count / repeat;
          history.push_back(time);
          sum += time;
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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", branch_toggle, target_toggle,
              min, sum / history.size(), max);
      fflush(fp);
    }
  }

  printf("Results are written to phr_branch_target_xor.csv\n");
  delete[] buffer;
  return 0;
}
