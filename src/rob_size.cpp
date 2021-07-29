#include "include/utils.h"
#include <stdio.h>

// ref:
// http://blog.stuffedcow.net/2013/05/measuring-rob-capacity/
// https://github.com/travisdowns/robsize
// use with generate_gadget tool

// defined in gen_rob_test()
typedef void (*gadget)(char **);
extern "C" {
extern gadget rob_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 10000;
  int repeat = 5;
  int min_size = 32;
  int max_size = 256;

  size_t buffer_size = 1024 * 1024 * 128; // 128 MB
  char **buffer = generate_random_pointer_chasing(buffer_size);
  printf("size,time(ns)\n");
  for (int size = min_size; size <= max_size; size++) {
    // warmup ICache
    rob_gadgets[size - min_size](buffer);
    // flush DCache
    for (size_t i = 0; i < buffer_size; i++) {
      ((volatile char *)buffer)[i];
    }

    uint64_t begin = get_time_ns();
    rob_gadgets[size - min_size](buffer);
    uint64_t elapsed = get_time_ns() - begin;
    printf("%d,%.2lf\n", size, (double)elapsed / loop_count / repeat);
  }
  delete[] buffer;
  return 0;
}