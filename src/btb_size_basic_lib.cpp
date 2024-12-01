#include "include/utils.h"
#include <assert.h>
#include <cstdint>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// generated in btb_size_basic_gen.cpp
// args: loop count
typedef void (*gadget)(size_t);
extern "C" {
extern gadget btb_size_basic_gadgets[];
}

void btb_size_basic(FILE *fp) {
  int loop_count = 100;
  // match gen_btb_test
  uint64_t min_size = 0, max_size = 0, max_product = 0, min_stride = 0,
           max_stride = 0;
  std::vector<uint64_t> mults;
  int num_patterns = 3;

  min_size = 2;
  max_size = 65536;
  max_product = 32768;
  min_stride = 4;
  max_stride = 8192;
  mults = {1, 3, 5, 7};

  bind_to_core();
  setup_perf_cycles();
  fprintf(fp, "pattern,size,stride,min,avg,max\n");
  int gadget_index = 0;
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (uint64_t stride = min_stride; stride <= max_stride; stride *= 2) {
      std::set<int> sizes;
      for (uint64_t size_base = min_size; size_base <= max_product / stride;
           size_base *= 2) {
        for (uint64_t mult : mults) {
          for (uint64_t size = size_base * mult - 1;
               size <= size_base * mult + 1 && size * stride <= max_product &&
               size <= max_size;
               size++) {
            sizes.insert(size);
          }
        }
      }

      for (uint64_t size : sizes) {
        gadget entry = btb_size_basic_gadgets[gadget_index];

        std::vector<double> history;
        int iterations = 30;
        history.reserve(iterations);

        double sum = 0;
        // run several times
        for (int i = 0; i < iterations; i++) {
          uint64_t begin = perf_read_cycles();
          entry(loop_count);
          uint64_t elapsed = perf_read_cycles() - begin;

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
        fprintf(fp, "%d,%ld,%ld,%.2lf,%.2lf,%.2lf\n", pattern, size, stride,
                min, sum / history.size(), max);
        fflush(fp);
      }
    }
  }
}
