#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern void if_width(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("if_width.csv", "w");
  assert(fp);
  if_width(fp);

  printf("Results are written to if_width.csv\n");
  return 0;
}
