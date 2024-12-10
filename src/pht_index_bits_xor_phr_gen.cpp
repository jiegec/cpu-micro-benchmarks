#include <cstdio>

// check index bit conflict xor PHR vs PHR
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  // jit only
  fclose(fp);
  return 0;
}
