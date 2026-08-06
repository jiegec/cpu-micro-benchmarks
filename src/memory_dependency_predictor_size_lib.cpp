#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// ref:
// http://blog.stuffedcow.net/2013/05/measuring-rob-capacity/
// https://github.com/travisdowns/robsize
// https://blog.stuffedcow.net/2014/01/x86-memory-disambiguation/
// use with generate_gadget tool

// defined in gen_rob_test()
// args: loop count
typedef char **(*gadget)(size_t, char *, char *);
extern "C" {
extern gadget memory_dependency_predictor_size_gadgets[];
}

void memory_dependency_predictor_size(FILE *fp) {
  int loop_count = 1000;
  // match gen_rob_test

  bind_to_core();
  setup_perf_cycles();
  int min_size = 1;
  int max_size = 128;
  int num_patterns = 5;

  int gadget_index = 0;
  char buffer[256] __attribute__((aligned(64)));
  fprintf(fp, "pattern,size,min,avg,max\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = perf_read_cycles();
        memory_dependency_predictor_size_gadgets[gadget_index](loop_count,
                                                               buffer, buffer);
        uint64_t elapsed = perf_read_cycles() - begin;

        // skip warmup
        if (i >= 10) {
          // cpi
          double time = (double)elapsed / size / loop_count;
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

      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", pattern, size, min,
              sum / history.size(), max);
      fflush(fp);
    }
  }
}
