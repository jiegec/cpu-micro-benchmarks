#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// generated in pht_tag_bits_xor_phr_gen.cpp
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget pht_tag_bits_xor_phr_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 5000;
  // match gen_pht_tag_bits_xor_phr_test
  int min_first_phr_bit = 0;
  int max_first_phr_bit = 35;
  int min_dummy_branches = 0;
  int max_dummy_branches = PHR_BRANCHES;

  bind_to_core();
  setup_perf_cond_branch_misses();
  FILE *fp = fopen("pht_tag_bits_xor_phr.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];
  for (int i = 0; i <= loop_count; i++) {
    buffer[i] = rand() % 4;
  }

  fprintf(fp, "target,first_phr_bit,dummy_branches,min,avg,max\n");
  int gadget_index = 0;
  for (int inject_target = 0; inject_target <= 1; inject_target++) {
    for (int first_phr_bit = min_first_phr_bit;
         first_phr_bit <= max_first_phr_bit; first_phr_bit++) {
      for (int dummy_branches = min_dummy_branches;
           dummy_branches <= max_dummy_branches; dummy_branches++) {
        std::vector<double> history;
        int iterations = 100;
        history.reserve(iterations);

        double sum = 0;
        // run several times
        for (int i = 0; i < iterations; i++) {
          uint64_t begin = perf_read_cond_branch_misses();
          pht_tag_bits_xor_phr_gadgets[gadget_index](loop_count, buffer);
          uint64_t elapsed = perf_read_cond_branch_misses() - begin;

          // skip warmup
          if (i >= 10) {
            // 1/8 branches
            double time = (double)elapsed / loop_count * 4;
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
        fprintf(fp, "%d,%d,%d,%.2lf,%.2lf,%.2lf\n", inject_target,
                first_phr_bit, dummy_branches, min, sum / history.size(), max);
        fflush(fp);
      }
    }
  }

  printf("Results are written to pht_tag_bits_xor_phr.csv\n");
  delete[] buffer;
  return 0;
}
