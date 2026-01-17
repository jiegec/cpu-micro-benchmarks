#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

extern void sched_size(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("sched_size.csv", "w");
  assert(fp);
  sched_size(fp);

  printf("Results are written to sched_size.csv\n");
  return 0;
}
