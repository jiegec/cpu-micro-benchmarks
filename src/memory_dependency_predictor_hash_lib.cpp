#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// defined in gen_rob_test()
// args: ptr1, ptr2
typedef char **(*gadget)(char *, char *);
extern "C" {
extern gadget memory_dependency_predictor_hash_gadgets[];
}

void memory_dependency_predictor_hash(FILE *fp) {
  // match gen_rob_test
  bind_to_core();
  setup_perf_cycles();
  int min_bit = 0;
  int max_bit = 21;
  int num_patterns = 1;

  int gadget_index = 0;
  char buffer1[256] __attribute__((aligned(64)));
  char buffer2[256] __attribute__((aligned(64)));
  fprintf(fp, "pattern,bit,min,avg,max\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int bit = min_bit; bit <= max_bit; bit++) {
      std::vector<double> history;
      int iterations = 5000;
      history.reserve(iterations);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = perf_read_cycles();
        memory_dependency_predictor_hash_gadgets[0](buffer1, buffer1);
        memory_dependency_predictor_hash_gadgets[gadget_index](buffer2,
                                                               buffer2 + 64);
        uint64_t elapsed = perf_read_cycles() - begin;

        // skip warmup
        if (i >= 10) {
          // cpi
          double time = (double)elapsed;
          history.push_back(time);
          sum += time;
        }
      }

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
      gadget_index++;

      if (bit == 0)
        continue;
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", pattern, bit, min,
              sum / history.size(), max);
      fflush(fp);
    }
  }
}
