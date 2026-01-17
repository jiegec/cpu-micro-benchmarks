#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// ref:
// https://zhuanlan.zhihu.com/p/720136752

// generated in if_width_gen.cpp
// args: loop count
typedef char **(*gadget)(size_t);
extern "C" {
extern gadget if_width_gadgets[];
}

void if_width(FILE *fp) {
  int loop_count = 100000;
  int min_size = 2;
  int max_size = 64;
  int min_pattern = 0;
  int max_pattern = 1;

  bind_to_core();
  setup_perf_cycles();

  int gadget_index = 0;
  fprintf(fp, "pattern,size,min,avg,max\n");
  for (int pattern = min_pattern; pattern <= max_pattern; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = perf_read_cycles();
        if_width_gadgets[gadget_index](loop_count);
        uint64_t elapsed = perf_read_cycles() - begin;

        // skip warmup
        if (i >= 10) {
          double time = (double)elapsed / loop_count;
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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", pattern, size, min,
              sum / history.size(), max);
      fflush(fp);
    }
  }
}
