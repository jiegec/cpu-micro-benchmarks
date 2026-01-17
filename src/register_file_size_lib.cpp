#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// generated in register_file_size_gen.cpp
// args: loop count
typedef char **(*gadget)(char ***, char ***, size_t);
extern "C" {
extern gadget register_file_size_gadgets[];
}

void register_file_size(FILE *fp) {
#ifdef GEM5
  int loop_count = 100;
#else
  int loop_count = 2000;
#endif
  // match gen_rob_test
  int min_size = 0;
  int max_size = 800;
#ifdef HOST_AARCH64
  int num_patterns = 5;
#else
  int num_patterns = 8;
#endif

#ifdef GEM5
  size_t buffer_size = 1024 * 1024 * 16; // 16 MB
#else
  size_t buffer_size = 1024 * 1024 * 256; // 256 MB
#endif
  char **buffer1 = generate_random_pointer_chasing(buffer_size);
  char **p1 = buffer1;
  char **buffer2 = generate_random_pointer_chasing(buffer_size);
  char **p2 = buffer2;
  bind_to_core();

  int gadget_index = 0;
  fprintf(fp, "pattern,size,min,avg,max\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
#ifdef GEM5
      if (size >= 64) {
        gadget_index++;
        continue;
      }
#endif
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = get_time();
        register_file_size_gadgets[gadget_index](&p1, &p2, loop_count);
        uint64_t elapsed = get_time() - begin;

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
