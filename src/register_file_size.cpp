#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

void register_file_size(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("register_file_size.csv", "w");
  assert(fp);
  register_file_size(fp);
  printf("Results are written to register_file_size.csv\n");
  return 0;
}
