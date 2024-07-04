#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// ref:
// http://blog.stuffedcow.net/2013/05/measuring-rob-capacity/
// https://github.com/travisdowns/robsize
// https://chipsandcheese.com/2023/10/27/cortex-x2-arm-aims-high/
// https://zhuanlan.zhihu.com/p/595585895
// use with generate_gadget tool

// defined in gen_btb_test()
// args: loop count
typedef void (*gadget)(size_t);
extern "C" {
extern gadget btb_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  // match gen_rob_test
  int min_size = 4;
  int max_size = 8192;
  int min_stride = 4;
  int max_stride = 128;

  bind_to_core();
  setup_time_or_cycles();
  FILE *fp = fopen("btb.csv", "w");
  assert(fp);

  fprintf(fp, "size,stride,min,avg,max\n");
  int gadget_index = 0;
  for (int size = min_size; size <= max_size; size = (size * 1.19 + 1)) {
    for (int stride = min_stride; stride <= max_stride; stride *= 2) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = get_time_or_cycles();
        btb_gadgets[gadget_index](loop_count);
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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", size, stride, min,
              sum / history.size(), max);
      fflush(fp);
    }
  }

  printf("Results are written to btb.csv\n");
  return 0;
}
