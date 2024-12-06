#ifdef __linux__
#include "include/utils.h"
#include <assert.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

// https://arxiv.org/pdf/2411.13900
// generated in find_branch_misses_pmu_gen.cpp
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget find_branch_misses_pmu_gadgets[];
}

int main(int argc, char *argv[]) {
  int num_patterns = 3;
  int loop_count = 1000;
  int min_counter = 0x0;
  int max_counter = 0x1000;
  const char *pattern_names[] = {
      "50% cond branch miss",
      "50% indirect branch miss",
      "50% cond + indirect branch miss",
  };

  bind_to_core();
  FILE *fp = fopen("find_branch_misses_pmu.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];
  for (int i = 0; i <= loop_count; i++) {
    buffer[i] = rand() % 2;
  }

  fprintf(fp, "pattern,counter,min,avg,max\n");
  int gadget_index = 0;
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int counter = min_counter; counter <= max_counter; counter++) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      double sum = 0;
      raw_perf_counter perf =
          setup_perf_common_failable(PERF_TYPE_RAW, counter);
      if (perf.fd < 0) {
        continue;
      }

      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = perf.read();
        find_branch_misses_pmu_gadgets[gadget_index](loop_count, buffer);
        uint64_t elapsed = perf.read() - begin;

        // skip warmup
        if (i >= 10) {
          double time = (double)elapsed / loop_count;
          history.push_back(time);
          sum += time;
        }
      }
      close(perf.fd);
      if (perf.page) {
        munmap(perf.page, getpagesize());
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
      if (max > 0.0)
        fprintf(fp, "%s,0x%x,%.2lf,%.2lf,%.2lf\n", pattern_names[pattern],
                counter, min, sum / history.size(), max);
      fflush(fp);
    }
    gadget_index++;
  }

  printf("Results are written to find_branch_misses_pmu.csv\n");
  delete[] buffer;
  return 0;
}
#else
#include <stdio.h>
int main(int argc, char *argv[]) {
  printf("Not supported\n");
  return 0;
}
#endif
