#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// ref:
// https://zhuanlan.zhihu.com/p/595585895

// generated in ras_size_gen.cpp
// args: loop count
typedef void (*gadget)(size_t);
extern "C" {
extern gadget ras_size_gadgets[];
}

void ras_size(FILE *fp) {
#ifdef GEM5
  int loop_count = 10;
#else
  int loop_count = 1000;
#endif
  // match gen_ras_test
  int min_size = 1;
  int max_size = 128;

  bind_to_core();
  setup_time_or_cycles();
  fprintf(fp, "size,min,avg,max\n");
  int gadget_index = 0;
  for (int size = min_size; size <= max_size; size++) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

    double sum = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      uint64_t begin = get_time_or_cycles();
      ras_size_gadgets[gadget_index](loop_count);
      uint64_t elapsed = get_time_or_cycles() - begin;

      // skip warmup
      if (i >= 10) {
        double time = (double)elapsed / loop_count / size;
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
    fprintf(fp, "%d,%.2lf,%.2lf,%.2lf\n", size, min, sum / history.size(), max);
    fflush(fp);
  }
  return;
}
