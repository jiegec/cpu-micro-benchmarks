#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <vector>

// ref:
// http://blog.stuffedcow.net/2013/05/measuring-rob-capacity/
// https://github.com/travisdowns/robsize
// use with generate_gadget tool

// defined in gen_rob_test()
typedef char **(*gadget)(char **);
extern "C" {
extern gadget rob_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  int repeat = 100;
  int min_size = 32;
  int max_size = 256;

  size_t buffer_size = 1024 * 1024 * 128; // 128 MB
  // use two pointer to hack prefetcher
  char **buffer = generate_random_pointer_chasing(buffer_size);
  char **p = buffer;
  printf("size,min,avg,max\n");
  for (int size = min_size; size <= max_size; size++) {
    std::vector<double> history;
    double sum = 0;
    // run several times
    for (int i = 0; i < 5; i++) {
      uint64_t begin = get_time_ns();
      p = rob_gadgets[size - min_size](p);
      uint64_t elapsed = get_time_ns() - begin;

      double time = (double)elapsed / loop_count / repeat;
      history.push_back(time);
      sum += time;
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
    printf("%d,%.2lf,%.2lf,%.2lf\n", size, min, sum / history.size(), max);
  }
  delete[] buffer;
  return 0;
}