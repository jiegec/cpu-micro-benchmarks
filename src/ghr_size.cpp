#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern void ghr_size(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("ghr_size.csv", "w");
  assert(fp);
  ghr_size(fp);
  printf("Results are written to ghr_size.csv\n");
  return 0;
}
