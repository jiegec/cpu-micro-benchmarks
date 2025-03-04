#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// generated in rob_size_gen.cpp
// args: buffer1, buffer2, loop count
typedef void (*gadget)(char ***, char ***, size_t);
extern "C" {
extern gadget rob_size_gadgets[];
}

int main(int argc, char *argv[]) {
#ifdef GEM5
  int loop_count = 10;
#else
  int loop_count = 1000;
#endif
  // match gen_rob_test
  int repeat = 20;
  int min_size = 1;
  int max_size = 1024;

  bind_to_core();
  setup_time_or_cycles();
  FILE *fp = fopen("../run_results/rob_size.csv", "w");
  assert(fp);

#ifdef GEM5
  size_t buffer_size = 1024 * 1024 * 16; // 16 MB
#else
  size_t buffer_size = 1024 * 1024 * 256; // 256 MB
#endif
  char **buffer1 = generate_random_pointer_chasing(buffer_size);
  char **p1 = buffer1;
  char **buffer2 = generate_random_pointer_chasing(buffer_size);
  char **p2 = buffer2;
  fprintf(fp, "size,min,avg,max\n");
  for (int size = min_size; size <= max_size; size++) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

    double sum = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      uint64_t begin = get_time_or_cycles();
      rob_size_gadgets[size - min_size](&p1, &p2, loop_count);
      uint64_t elapsed = get_time_or_cycles() - begin;

      // skip warmup
      if (i >= 10) {
        double time = (double)elapsed / loop_count / repeat;
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
    fprintf(fp, "%d,%.2lf,%.2lf,%.2lf\n", size, min, sum / history.size(), max);
    fflush(fp);
  }

  printf("Results are written to rob_size.csv\n");
  delete[] buffer1;
  delete[] buffer2;
  return 0;
}
