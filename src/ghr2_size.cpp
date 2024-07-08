#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// use with generate_gadget tool

// defined in gen_ghr2_test()
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget ghr2_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  // match gen_ghr2_test
#if defined(__x86_64__)
  int min_branch_align = 13;
  int max_branch_align = 19;
  int min_target_align = 3;
  int max_target_align = 8;
#else
  int min_branch_align = 10;
  int max_branch_align = 17;
  int min_target_align = 3;
  int max_target_align = 10;
#endif

  bind_to_core();
  setup_perf_branch_misses();
  FILE *fp = fopen("ghr2_size.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];
  for (int i = 0; i <= loop_count; i++) {
    buffer[i] = rand() % 2;
  }

  fprintf(fp, "branch,target,min,avg,max\n");
  int gadget_index = 0;
  int repeat = 2; // two branches
  for (int branch_align = min_branch_align; branch_align <= max_branch_align;
       branch_align++) {
    for (int target_align = min_target_align; target_align <= max_target_align;
         target_align++) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = perf_read_branch_misses();
        ghr2_gadgets[gadget_index](loop_count, buffer);
        uint64_t elapsed = perf_read_branch_misses() - begin;
        elapsed -=
            loop_count; // always one mis-prediction from last jnz dummy_target

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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", branch_align, target_align, min,
              sum / history.size(), max);
      fflush(fp);
    }
  }

  printf("Results are written to ghr2_size.csv\n");
  delete[] buffer;
  return 0;
}
