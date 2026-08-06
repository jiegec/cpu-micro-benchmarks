#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int num_patterns = 10;
  // args: ptr1, ptr2
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    fprintf(fp, ".global memory_dependency_predictor_reverse_%d\n", pattern);
    fprintf(fp, ".balign 64\n");
    fprintf(fp, "memory_dependency_predictor_reverse_%d:\n", pattern);
#if defined(HOST_AMD64)
    fprintf(fp, "\tlfence\n");
    // delay store address
    for (int i = 0; i < 20; i++) {
      fprintf(fp, "\timulq $1, %%rdi\n");
    }
    // store
    fprintf(fp, "\tmovq $0x1234, 0(%%rdi)\n");
    // load
    fprintf(fp, "\tmovq 0(%%rsi), %%rcx\n");
    // increase penalty
    for (int i = 0; i < 20; i++) {
      fprintf(fp, "\timulq $1, %%rcx\n");
    }

    fprintf(fp, "\tlfence\n");
    fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
    fprintf(fp, "\tdsb ish\n");
    fprintf(fp, "\tisb\n");
    fprintf(fp, "\tmov x3, #1\n");
    // delay store address
    for (int i = 0; i < 20; i++) {
      fprintf(fp, "\tmul x0, x0, x3\n");
    }
    // store
    fprintf(fp, "\tstr x3, [x0]\n");
    // load
    fprintf(fp, "\tldr x2, [x1]\n");
    // increase penalty
    for (int i = 0; i < 20; i++) {
      fprintf(fp, "\tmul x2, x2, x3\n");
    }

    fprintf(fp, "\tdsb ish\n");
    fprintf(fp, "\tisb\n");
    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "memory_dependency_predictor_reverse_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    add_gadget(fp, "memory_dependency_predictor_reverse_%d", pattern);
  }
  return 0;
}
