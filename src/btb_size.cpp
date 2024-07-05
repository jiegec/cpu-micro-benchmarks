#include "include/utils.h"
#include <assert.h>
#include <set>
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
  int loop_count = 2000;
  // match gen_btb_test
  int min_size = 4;
  int max_product = 262144;
  int min_stride = 4;
  int max_stride = 65536;

  bind_to_core();
  setup_time_or_cycles();
  FILE *fp = fopen("btb_size.csv", "w");
  assert(fp);

  fprintf(fp, "size,stride,min,avg,max\n");
  int gadget_index = 0;
  for (int stride = min_stride; stride <= max_stride; stride *= 2) {
    std::set<int> sizes;
    for (int size_base = min_size; size_base <= max_product / stride;
         size_base *= 2) {
      for (int size_mid = size_base; size_mid < size_base * 2;
           size_mid *= 1.25992105) {
        for (int size = size_mid - 1; size <= size_mid + 1; size++) {
          if (sizes.find(size) != sizes.end()) {
            continue;
          }
          sizes.insert(size);

          std::vector<double> history;
          int iterations = 15;
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
    }
  }

  printf("Results are written to btb_size.csv\n");
  return 0;
}
