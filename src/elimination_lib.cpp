#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// generated in elimination_gen.cpp
// args: loop count
typedef char **(*gadget)(size_t);
extern "C" {
extern gadget elimination_gadgets[];
}

void elimination(FILE *fp) {
  int loop_count = 10000;
  // match gen_rob_test

  bind_to_core();
  setup_perf_instructions_per_cycle();
  int num_patterns = 16;
  const char *pattern_names[] = {"int dependent add",
                                 "int independent add",
                                 "int dependent mov",
                                 "int independent mov",
                                 "int dependent zero via xor",
                                 "int dependent zero via sub",
                                 "int independent zero via mov",
                                 "int independent one via mov",
                                 "int independent two via mov",
                                 "int independent 1024 via mov",
                                 "vec dependent mov",
                                 "vec independent mov",
                                 "vec dependent zero via xor",
                                 "vec dependent zero via sub",
                                 "vec independent zero via mov",
                                 "nop"};

  int gadget_index = 0;
  fprintf(fp, "pattern,min,avg,max\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    std::vector<double> history;
    int iterations = 100;
    history.reserve(iterations);

    double sum = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      perf_begin_instructions_per_cycle();
      elimination_gadgets[gadget_index](loop_count);
      counter_per_cycle elapsed = perf_end_instructions_per_cycle();

      // skip warmup
      if (i >= 10) {
        double time = (double)elapsed.counter / elapsed.cycles;
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
    fprintf(fp, "%s,%.2lf,%.2lf,%.2lf\n", pattern_names[pattern], min,
            sum / history.size(), max);
    fflush(fp);
  }
}
