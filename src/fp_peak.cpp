#include "include/utils.h"
#include <assert.h>
#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

// defined in fp_peak_gen.cpp
// args: loop count
typedef void (*gadget)(size_t);
extern "C" {
extern gadget fp_peak_gadgets[];
}

int main(int argc, char *argv[]) {
  // match fp_peak_gen.cpp
  int repeat = 1000;
  int loop_count = 1000;

#ifdef HOST_AARCH64
  int num_patterns = 6;
  char patterns[][20] = {
      "32-bit SP FMADD",  "64-bit DP FMADD", "128-bit SP ASIMD",
      "128-bit DP ASIMD", "xxxx-bit SP SVE", "xxxx-bit DP SVE",
  };

  int coef[] = {
      32 / 32 * 2,  // 32-bit SP
      64 / 64 * 2,  // 64-bit DP
      128 / 32 * 2, // 128-bit SP
      128 / 64 * 2, // 128-bit DP
      0,            // ?-bit SP
      0,            // ?-bit DP
  };
#else
  int num_patterns = 4;
  const char *patterns[] = {
      "256-bit SP FMA",
      "256-bit DP FMA",
      "512-bit SP AVX512F",
      "512-bit DP AVX512F",
  };

  int coef[] = {
      256 / 32 * 2, // 256-bit SP
      256 / 64 * 2, // 256-bit DP
      512 / 32 * 2, // 512-bit SP
      512 / 64 * 2, // 512-bit DP
  };
#endif

  bind_to_core();
  setup_perf_cycles();
  FILE *fp = fopen("fp_peak.csv", "w");
  assert(fp);

  int gadget_index = 0;
  fprintf(fp, "pattern,min,avg,max\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

#ifdef HOST_AARCH64
    // read sve length in runtime
    if (pattern == 4) {
      uint64_t len = 0;
      asm __volatile__(".arch armv9-a+sve\ncntw %0" : "=r"(len));
      sprintf(patterns[pattern], "%ld-bit SP SVE", len * 32);
      coef[pattern] = len * 2;
    } else if (pattern == 5) {
      uint64_t len = 0;
      asm __volatile__(".arch armv9-a+sve\ncntd %0" : "=r"(len));
      sprintf(patterns[pattern], "%ld-bit DP SVE", len * 64);
      coef[pattern] = len * 2;
    }
#endif

    double sum = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      uint64_t begin = perf_read_cycles();
      fp_peak_gadgets[gadget_index](loop_count);
      uint64_t elapsed = perf_read_cycles() - begin;

      // skip warmup
      if (i >= 10) {
        double time =
            (double)coef[pattern] / ((double)elapsed / loop_count / repeat);
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

    fprintf(fp, "%s,%.2lf,%.2lf,%.2lf\n", patterns[pattern], min,
            sum / history.size(), max);
    fflush(fp);

    gadget_index++;
  }

  printf("Results are written to fp_peak.csv\n");
  return 0;
}
