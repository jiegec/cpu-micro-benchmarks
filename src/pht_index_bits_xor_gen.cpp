#include "include/utils.h"

// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf
// https://arxiv.org/pdf/2411.13900
// check index bit conflict xor PC vs PHR
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  // use jit exclusively
  fclose(fp);
  return 0;
}
