#include "include/utils.h"
#include <assert.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

// defined in bp_size_gen.cpp
// args: loop count, history buffer, history length
typedef void (*gadget)(size_t, uint32_t **, size_t);
extern "C" {
extern gadget bp_size_gadgets[];
}

int main(int argc, char *argv[]) {
  // match bp_size_gen.cpp
  int min_size = 1;
  int max_size = 2048;
  int min_history_len = 1;
  int max_history_len = 65536;
  std::vector<int> mult = {1,  3,  5,  7,  9,  11, 13, 15, 17, 19, 21, 23,
                           25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47};
  std::set<int> history_lens;
  for (int history_len = min_history_len; history_len <= max_history_len;
       history_len *= 2) {
    for (int m : mult) {
      if (m * history_len <= max_history_len) {
        history_lens.insert(m * history_len);
      }
    }
  }
  int loop_count = 200000;

  int opt;
  bool record_cond_branch_misses = false;
  while ((opt = getopt(argc, argv, "m")) != -1) {
    switch (opt) {
    case 'm':
      record_cond_branch_misses = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-m]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  bind_to_core();
  setup_time_or_cycles();
  if (record_cond_branch_misses) {
    setup_perf_cond_branch_misses();
  }
  FILE *fp = fopen("bp_size.csv", "w");
  assert(fp);

  if (record_cond_branch_misses) {
    fprintf(fp, "size,history,min,avg,max,miss,cap\n");
  } else {
    fprintf(fp, "size,history,min,avg,max\n");
  }
  int gadget_index = 0;
  for (int size = min_size; size <= max_size; size *= 2) {
    for (int history_len : history_lens) {

      // generate random history
      uint32_t **patterns = new uint32_t *[size];
      for (int i = 0; i < size; i++) {
        patterns[i] = new uint32_t[history_len];
        for (int j = 0; j < history_len; j++) {
          patterns[i][j] = rand() % 2;
        }
      }

      std::vector<double> history;
      int iterations = 5;
      history.reserve(iterations);

      double sum = 0;
      double sum_cond_branch_misses = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = get_time_or_cycles();
        uint64_t begin_cond_branch_misses = 0;
        if (record_cond_branch_misses) {
          begin_cond_branch_misses = perf_read_cond_branch_misses();
        }
        bp_size_gadgets[gadget_index](loop_count, patterns, history_len);
        uint64_t elapsed = get_time_or_cycles() - begin;
        uint64_t elapsed_cond_branch_misses = 0;
        if (record_cond_branch_misses) {
          elapsed_cond_branch_misses =
              perf_read_cond_branch_misses() - begin_cond_branch_misses;
        }

        // skip warmup
        if (i >= 1) {
          double time = (double)elapsed / loop_count / size;
          history.push_back(time);
          sum += time;

          double cond_branch_misses =
              (double)elapsed_cond_branch_misses / loop_count;
          sum_cond_branch_misses += cond_branch_misses;
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

      if (record_cond_branch_misses) {
        // estimate capacity
        // assume the branch predictor has m-entry in total
        // we predict n branches, each branch has k history
        // misprediction rate should be: (n*k-m)/2/n/k=0.5-m/(2nk)
        // we can compute misprediction rate via: cond_branch_miss / n
        // so m=(0.5-cond_branch_miss/n)*2*n*k
        // do not compute this if we are not looping enough times
        fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf,%lf,%.2lf\n", size, history_len,
                min, sum / history.size(), max,
                sum_cond_branch_misses / history.size(),
                loop_count >= size * history_len * 2
                    ? ((0.5 - sum_cond_branch_misses / history.size() / size) *
                       2.0 * size * history_len)
                    : 0);
      } else {
        fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", size, history_len, min,
                sum / history.size(), max);
      }
      fflush(fp);

      for (int i = 0; i < size; i++) {
        delete[] patterns[i];
      }
      delete[] patterns;
    }
    gadget_index++;
  }

  printf("Results are written to bp_size.csv\n");
  return 0;
}
