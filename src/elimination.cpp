#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

extern void elimination(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("../run_results/elimination.csv", "w");
  assert(fp);
  elimination(fp);

  printf("Results are written to elimination.csv\n");
  return 0;
}
