#include "include/utils.h"
#include <assert.h>
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <utility>
#include <vector>

extern void instruction_latency(FILE *fp);
extern bool instruction_latency_use_perf;
extern int instruction_latency_loop_count;
int main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "n:p")) != -1) {
    switch (opt) {
    case 'n':
      sscanf(optarg, "%d", &instruction_latency_loop_count);
      break;
    case 'p':
      instruction_latency_use_perf = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-p]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  FILE *fp = fopen("instruction_latency.csv", "w");
  assert(fp);
  instruction_latency(fp);
  printf("Result written to instruction_latency.csv\n");
  return 0;
}
