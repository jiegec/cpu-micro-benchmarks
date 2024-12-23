#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int num_patterns = 16;
  int repeat = 1000;
  // args: loop count
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    fprintf(fp, ".global elimination_%d\n", pattern);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "elimination_%d:\n", pattern);

#if defined(HOST_AMD64)
    fprintf(fp, "\tpushq %%r12\n");
    fprintf(fp, "\tpushq %%r13\n");
    fprintf(fp, "\tpushq %%r14\n");
    fprintf(fp, "\tpushq %%r15\n");
    fprintf(fp, "\tpushq %%rbx\n");

    fprintf(fp, "\t1:\n");

    for (int i = 0; i < repeat; i++) {
      if (pattern == 0) {
        // int dependent add
        // r8 -> r9 -> ... -> r15 -> r8
        fprintf(fp, "\tadd %%r%d, %%r%d\n", 8 + (i % 8), 8 + ((i + 1) % 8));
      } else if (pattern == 1) {
        // int independent add
        fprintf(fp, "\tadd %%rax, %%r%d\n", 8 + (i % 8));
      } else if (pattern == 2) {
        // int dependent mov
        // r8 -> r9 -> ... -> r15 -> r8
        fprintf(fp, "\tmov %%r%d, %%r%d\n", 8 + (i % 8), 8 + ((i + 1) % 8));
      } else if (pattern == 3) {
        // int independent mov
        // r8 -> r9, ... r15
        fprintf(fp, "\tmov %%r8, %%r%d\n", 9 + (i % 7));
      } else if (pattern == 4) {
        // int xor zero
        fprintf(fp, "\txor %%r8, %%r8\n");
      } else if (pattern == 5) {
        // sub zero
        fprintf(fp, "\tsub %%r8, %%r8\n");
      } else if (pattern == 6) {
        // mov zero
        fprintf(fp, "\tmov $0, %%r%d\n", 8 + (i % 8));
      } else if (pattern == 7) {
        // mov one
        fprintf(fp, "\tmov $1, %%r%d\n", 8 + (i % 8));
      } else if (pattern == 8) {
        // mov two
        fprintf(fp, "\tmov $2, %%r%d\n", 8 + (i % 8));
      } else if (pattern == 9) {
        // mov 1024
        fprintf(fp, "\tmov $1024, %%r%d\n", 8 + (i % 8));
      } else if (pattern == 10) {
        // dependent mov
        // xmm8 -> xmm9 -> ... -> xmm15
        fprintf(fp, "\tmovdqu %%xmm%d, %%xmm%d\n", 8 + (i % 8),
                8 + ((i + 1) % 8));
      } else if (pattern == 11) {
        // independent mov
        // xmm8 -> xmm9, xmm15
        fprintf(fp, "\tmovdqu %%xmm8, %%xmm%d\n", 9 + (i % 7));
      } else if (pattern == 12) {
        // xor zero vec
        fprintf(fp, "\txorps %%xmm8, %%xmm8\n");
      } else if (pattern == 13) {
        // sub zero vec
        fprintf(fp, "\tsubps %%xmm8, %%xmm8\n");
      } else if (pattern == 14) {
        // vec mov zero
        // no available on x86
      } else if (pattern == 15) {
        // nop
        fprintf(fp, "\tnop\n");
      }
    }

    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tjne 1b\n");

    // return
    fprintf(fp, "\tpopq %%rbx\n");
    fprintf(fp, "\tpopq %%r15\n");
    fprintf(fp, "\tpopq %%r14\n");
    fprintf(fp, "\tpopq %%r13\n");
    fprintf(fp, "\tpopq %%r12\n");
    fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
    fprintf(fp, "\t1:\n");

    for (int i = 0; i < repeat; i++) {
      if (pattern == 0) {
        // int dependent add
        // r8 -> r9 -> ... -> r15 -> r8
        fprintf(fp, "\tadd x%d, x%d, x%d\n", 8 + ((i + 1) % 8),
                8 + ((i + 1) % 8), 8 + (i % 8));
      } else if (pattern == 1) {
        // int independent add
        fprintf(fp, "\tadd x%d, x1, x2\n", 8 + (i % 8));
      } else if (pattern == 2) {
        // int dependent mov
        // r8 -> r9 -> ... -> r15 -> r8
        fprintf(fp, "\tmov x%d, x%d\n", 8 + ((i + 1) % 8), 8 + (i % 8));
      } else if (pattern == 3) {
        // independent mov
        // r8 -> r9, ... r15
        fprintf(fp, "\tmov x%d, x8\n", 9 + (i % 7));
      } else if (pattern == 4) {
        // int xor zero
        // r8 -> r9 -> ... -> r15 -> r8
        fprintf(fp, "\teor x%d, x%d, x%d\n", 8 + ((i + 1) % 8), 8 + (i % 8),
                8 + (i % 8));
      } else if (pattern == 5) {
        // sub zero
        // r8 -> r9 -> ... -> r15 -> r8
        fprintf(fp, "\tsub x%d, x%d, x%d\n", 8 + ((i + 1) % 8), 8 + (i % 8),
                8 + (i % 8));
      } else if (pattern == 6) {
        // mov zero
        fprintf(fp, "\tmov x%d, #0\n", 8 + (i % 8));
      } else if (pattern == 7) {
        // mov one
        fprintf(fp, "\tmov x%d, #1\n", 8 + (i % 8));
      } else if (pattern == 8) {
        // mov two
        fprintf(fp, "\tmov x%d, #2\n", 8 + (i % 8));
      } else if (pattern == 9) {
        // mov 1024
        fprintf(fp, "\tmov x%d, #1024\n", 8 + (i % 8));
      } else if (pattern == 10) {
        // vec dependent mov
        // v0 -> v1 -> ... -> v7
        fprintf(fp, "\tmov v%d.16b, v%d.16b\n", ((i + 1) % 8), (i % 8));
      } else if (pattern == 11) {
        // vec independent mov
        // v0 -> v1, v7
        fprintf(fp, "\tmov v%d.16b, v8.16b\n", 1 + (i % 7));
      } else if (pattern == 12) {
        // vec xor zero vec
        // v0 -> v1 -> ... -> v7
        fprintf(fp, "\teor v%d.16b, v%d.16b, v%d.16b\n", ((i + 1) % 8), (i % 8),
                (i % 8));
      } else if (pattern == 13) {
        // vec sub zero vec
        // v0 -> v1 -> ... -> v7
        fprintf(fp, "\tsub v%d.16b, v%d.16b, v%d.16b\n", ((i + 1) % 8), (i % 8),
                (i % 8));
      } else if (pattern == 14) {
        // vec mov zero
        fprintf(fp, "\tmovi v%d.4s, 0\n", ((i + 1) % 8));
      } else if (pattern == 15) {
        // nop
        fprintf(fp, "\tnop\n");
      }
    }

    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "elimination_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    add_gadget(fp, "elimination_%d", pattern);
  }
  return 0;
}
