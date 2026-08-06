#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern void memory_dependency_predictor_hash(FILE *fp);
int main(int argc, char *argv[]) {
  FILE *fp = fopen("memory_dependency_predictor_hash.csv", "w");
  assert(fp);
  memory_dependency_predictor_hash(fp);

  printf("Results are written to memory_dependency_predictor_hash.csv\n");
  return 0;
}
