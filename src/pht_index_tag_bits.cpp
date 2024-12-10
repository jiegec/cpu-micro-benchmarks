#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// https://arxiv.org/pdf/2411.13900
// defined in gen_pht_index_tag_bits_test()
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget pht_index_tag_bits_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  // match gen_pht_index_tag_bits_test
  int min_branch_align = 2;
#ifdef __APPLE__
  // cannot surpass page size
  int max_branch_align = 13;
#else
  int max_branch_align = 19;
#endif

  bind_to_core();
#ifdef NO_COND_BRANCH_MISSES
  setup_perf_branch_misses();
#else
  setup_perf_cond_branch_misses();
#endif
  FILE *fp = fopen("pht_index_tag_bits.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "align,min,avg,max\n");
  int gadget_index = 0;
  for (int branch_align = min_branch_align; branch_align <= max_branch_align;
       branch_align++) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

    double sum = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      for (int i = 0; i <= loop_count; i++) {
        buffer[i] = rand() % 2;
      }

#ifdef NO_COND_BRANCH_MISSES
      uint64_t begin = perf_read_branch_misses();
#else
      uint64_t begin = perf_read_cond_branch_misses();
#endif
      pht_index_tag_bits_gadgets[gadget_index](loop_count, buffer);
#ifdef NO_COND_BRANCH_MISSES
      uint64_t elapsed = perf_read_branch_misses() - begin;
#else
      uint64_t elapsed = perf_read_cond_branch_misses() - begin;
#endif

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
    fprintf(fp, "%d,%.2lf,%.2lf,%.2lf\n", branch_align, min,
            sum / history.size(), max);
    fflush(fp);
  }

  printf("Results are written to pht_index_tag_bits.csv\n");
  delete[] buffer;
  return 0;
}
