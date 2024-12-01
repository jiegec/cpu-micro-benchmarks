#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int repeat = 20;
  int min_size = 1;
  int max_size = 1024;
  // args: loop count, buffer
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    fprintf(fp, ".global rob_size_%d\n", size);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "rob_size_%d:\n", size);
#ifdef HOST_AARCH64
    // int sqrt_count = 8;
    fprintf(fp, "\tldr x3, [x0]\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tldr x3, [x3]\n");
      // use sqrt if necessary
      // for (int j = 0; j < sqrt_count; j++) {
      //   fprintf(fp, "\tfsqrt d0, d0\n");
      // }
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
    }
    fprintf(fp, "\tsubs x2, x2, #1\n");
    fprintf(fp, "\tbne 1b\n");
    fprintf(fp, "\tstr x3, [x0]\n");
    fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
    fprintf(fp, "\tmovq 0(%%rdi), %%r8\n");
    fprintf(fp, "\tmovq 0(%%rsi), %%r9\n");
    fprintf(fp, "\tmovq %%rdx, %%rax\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tmovq (%%r8), %%r8\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
      fprintf(fp, "\tmovq (%%r9), %%r9\n");
      // forbit further speculation
      fprintf(fp, "\tlfence\n");
      fprintf(fp, "\tmfence\n");
    }
    fprintf(fp, "\tsubl $1, %%eax\n");
    fprintf(fp, "\tjne 1b\n");
    fprintf(fp, "\tmovq %%r8, 0(%%rdi)\n");
    fprintf(fp, "\tmovq %%r9, 0(%%rsi)\n");
    fprintf(fp, "\tret\n");
#elif defined(__loongarch__)
    fprintf(fp, "\tld.d $a3, $a0, 0\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tld.d $a3, $a3, 0\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
    }
    fprintf(fp, "\taddi.d $a2, $a2, -1\n");
    fprintf(fp, "\tbne $a2, $zero, 1b\n");
    fprintf(fp, "\tst.d $a3, $a0, 0\n");
    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "rob_size_gadgets");
  for (int size = min_size; size <= max_size; size++) {
    add_gadget(fp, "rob_size_%d", size);
  }
  return 0;
}
