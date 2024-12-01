#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern void btb_size_basic(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("btb_size_basic.csv", "w");
  assert(fp);
  btb_size_basic(fp);
  printf("Results are written to btb_size_basic.csv\n");
  return 0;
}
