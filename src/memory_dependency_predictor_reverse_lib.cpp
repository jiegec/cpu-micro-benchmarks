#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// ref:
// Rage Against the Machine Clear: A Systematic Analysis of Machine Clears and
// Their Implications for Transient Execution Attacks
// use with generate_gadget tool

// defined in gen_rob_test()
// args: ptr1, ptr2
typedef char **(*gadget)(char *, char *);
extern "C" {
extern gadget memory_dependency_predictor_reverse_gadgets[];
}

struct pattern {
  int max_index;
  std::vector<int> gadget_index;
  std::vector<int> allow;
};

typedef uint64_t (*perf_read)();

void memory_dependency_predictor_reverse(FILE *fp) {
  bind_to_core();
  setup_perf_cycles();
#if defined(HOST_AMD64) || defined(APPLE_SILICON)
  setup_perf_machine_clear();
#endif
  int repeat = 5000;

  // pattern 1: 100N-20Y-10N
  // 0-99: disallow bypass
  // 100-119: allow bypass
  // 120-129: disallow bypass
  pattern pattern1;
  pattern1.max_index = 130;
  for (int index = 0; index < pattern1.max_index; index++) {
    pattern1.gadget_index.push_back(0);
    pattern1.allow.push_back(index >= 100 && index < 120);
  }

  // pattern 2: 100N-20Y-1N-19Y
  // 0-99: disallow bypass
  // 100-119: allow bypass
  // 120: disallow bypass
  // 121-139: allow bypass
  pattern pattern2;
  pattern2.max_index = 140;
  for (int index = 0; index < pattern2.max_index; index++) {
    pattern2.gadget_index.push_back(0);
    pattern2.allow.push_back((index >= 100 && index < 120) ||
                             (index >= 121 && index < 140));
  }

  // pattern 3: 100N-20Y-1N-9Y-1N-9Y
  // 0-99: disallow bypass
  // 100-119: allow bypass
  // 120: disallow bypass
  // 121-129: allow bypass
  // 130: disallow bypass
  // 131-139: allow bypass
  pattern pattern3;
  pattern3.max_index = 140;
  for (int index = 0; index < pattern3.max_index; index++) {
    pattern3.gadget_index.push_back(0);
    pattern3.allow.push_back(index >= 100 && index != 120 && index != 130);
  }

  // pattern 4: 1000Y-10N with different load
  pattern pattern4;
  pattern4.max_index = 1010;
  for (int index = 0; index < pattern4.max_index; index++) {
    pattern4.gadget_index.push_back((index < 1000) ? index % 10
                                                   : (index - 1000));
    pattern4.allow.push_back(index < 1000);
  }

  // pattern 5: 100Y-20N-20Y
  // 0-99: allow bypass
  // 100-119: disallow bypass
  // 120-139: allow bypass
  pattern pattern5;
  pattern5.max_index = 140;
  for (int index = 0; index < pattern5.max_index; index++) {
    pattern5.gadget_index.push_back(0);
    pattern5.allow.push_back(index < 100 || index >= 120);
  }

  std::vector<pattern> patterns;
  patterns.push_back(pattern1);
  patterns.push_back(pattern2);
  patterns.push_back(pattern3);
  patterns.push_back(pattern4);
  patterns.push_back(pattern5);

  char buffer[256] __attribute__((aligned(64)));
  fprintf(fp, "pattern,allow,index,min,avg,max,perf\n");
  for (int pattern = 0; pattern < (int)patterns.size(); pattern++) {
#if defined(HOST_AMD64) || defined(APPLE_SILICON)
    int max_perf = 2;
#else
    int max_perf = 1; // no machine clear
#endif
    for (int perf = 0; perf < max_perf; perf++) {
      perf_read read_func =
          perf == 0 ? perf_read_cycles : perf_read_machine_clear;

      std::vector<std::vector<double>> history;
      struct pattern &cur_pattern = patterns[pattern];
      history.resize(cur_pattern.max_index);
      for (int iteration = 0; iteration < repeat; iteration++) {
        for (int index = 0; index < cur_pattern.max_index; index++) {
          uint64_t begin = read_func();
          char *p1 = buffer;
          char *p2 = cur_pattern.allow[index] ? (buffer + 64) : buffer;
          memory_dependency_predictor_reverse_gadgets[cur_pattern
                                                          .gadget_index[index]](
              p1, p2);
          uint64_t elapsed = read_func() - begin;

          // cpi
          history[index].push_back((double)elapsed);
        }
      }

      for (int index = 0; index < cur_pattern.max_index; index++) {
        double min = history[index][0];
        double max = history[index][0];
        double sum = 0;
        for (size_t i = 0; i < history[index].size(); i++) {
          sum += history[index][i];
          if (min > history[index][i]) {
            min = history[index][i];
          }
          if (max < history[index][i]) {
            max = history[index][i];
          }
        }

        fprintf(fp, "%d,%s,%d,%.2lf,%.2lf,%.2lf,%s\n", pattern,
                cur_pattern.allow[index] ? "Y" : "N", index, min, sum / repeat,
                max, perf == 0 ? "cycles" : "machine_clear");
        fflush(fp);
      }
    }
  }
}
