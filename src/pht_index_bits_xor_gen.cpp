#include "include/utils.h"

// check index bit conflict xor PC vs PHR
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  // use jit exclusively
  fclose(fp);
  return 0;
}
