#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

extern void ras_size(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("ras_size.csv", "w");
  assert(fp);
  ras_size(fp);
  printf("Results are written to ras_size.csv\n");
  return 0;
}
