#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern void memory_dependency_predictor_size(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("memory_dependency_predictor_size.csv", "w");
  assert(fp);
  memory_dependency_predictor_size(fp);

  printf("Results are written to memory_dependency_predictor_size.csv\n");
  return 0;
}
