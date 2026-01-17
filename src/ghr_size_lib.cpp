#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// generated in ghr_size_gen.cpp
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget ghr_size_gadgets[];
}

void ghr_size(FILE *fp) {
  int loop_count = 1000;
  // match gen_ghr_test
  int repeat = 2;
  int min_size = 1;
  int max_size = 1024;

  bind_to_core();
  setup_perf_branch_misses();
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "size,min,avg,max\n");
  int gadget_index = 0;
  for (int size = min_size; size <= max_size; size++) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

    double sum = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      for (int i = 0; i <= loop_count; i++) {
        buffer[i] = rand() % 2;
      }
      uint64_t begin = perf_read_branch_misses();

      ghr_size_gadgets[gadget_index](loop_count, buffer);

      uint64_t elapsed = perf_read_branch_misses() - begin;

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
    fprintf(fp, "%d,%.2lf,%.2lf,%.2lf\n", size, min, sum / history.size(), max);
    fflush(fp);
  }
  delete[] buffer;
}
