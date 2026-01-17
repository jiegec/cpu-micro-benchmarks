#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern void fetch_bandwidth(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("fetch_bandwidth.csv", "w");
  assert(fp);
  fetch_bandwidth(fp);

  printf("Results are written to fetch_bandwidth.csv\n");
  return 0;
}
