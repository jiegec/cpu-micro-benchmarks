#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int num_patterns = 1;
  int min_bit = 0;
  int max_bit = 21;
  // args: ptr1, ptr2
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int bit = min_bit; bit <= max_bit; bit++) {
      fprintf(fp, ".global memory_dependency_predictor_hash_%d_%d\n", pattern,
              bit);
      fprintf(fp, ".balign %d\n", (1 << (max_bit + 1)));
#if defined(HOST_AMD64)
      if (bit > 0) {
        for (int i = 0; i < (1 << bit); i++) {
          fprintf(fp, "\tnop\n");
        }
      }
      fprintf(fp, "memory_dependency_predictor_hash_%d_%d:\n", pattern, bit);
      fprintf(fp, "\tlfence\n");
      if (pattern == 0) {
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
      }

      fprintf(fp, "\tlfence\n");
      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      if (bit > 0) {
        for (int i = 0; i < (1 << bit) / 4; i++) {
          fprintf(fp, "\tnop\n");
        }
      }
      fprintf(fp, "memory_dependency_predictor_hash_%d_%d:\n", pattern, bit);
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
  }

  define_gadgets_array(fp, "memory_dependency_predictor_hash_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int bit = min_bit; bit <= max_bit; bit++) {
      add_gadget(fp, "memory_dependency_predictor_hash_%d_%d", pattern, bit);
    }
  }
  return 0;
}
