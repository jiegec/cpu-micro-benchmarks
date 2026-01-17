#include "include/utils.h"
#include <assert.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// generated in fetch_bandwidth_gen.cpp
// args: loop count
typedef char **(*gadget)(size_t);
extern "C" {
extern gadget fetch_bandwidth_gadgets[];
}

void fetch_bandwidth(FILE *fp) {
  int loop_count = 10000;

  bind_to_core();
  setup_perf_instructions_per_cycle();
  int min_size = 1024;
  int max_size = 1048576;
  std::vector<int> mults = {1, 3, 5, 7, 9};
  std::set<int> sizes;
  for (int size = min_size; size <= max_size; size *= 2) {
    for (int mult : mults) {
      if (size * mult <= max_size) {
        sizes.insert(size * mult);
      }
    }
  }

  int gadget_index = 0;
  fprintf(fp, "size,min,avg,max\n");
  for (int size : sizes) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

    double sum = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      perf_begin_instructions_per_cycle();
      fetch_bandwidth_gadgets[gadget_index](loop_count);
      counter_per_cycle elapsed = perf_end_instructions_per_cycle();

      // skip warmup
      if (i >= 10) {
        double time = (double)elapsed.counter / elapsed.cycles;
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
}
