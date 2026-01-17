#include "include/utils.h"

// https://github.com/clamchowder/Microbenchmarks/blob/master/AsmGen/tests/IntRfTest.cs
// https://github.com/clamchowder/Microbenchmarks/blob/master/AsmGen/tests/FpRfTest.cs
// https://github.com/clamchowder/Microbenchmarks/blob/master/AsmGen/tests/FlagRfTest.cs
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 0;
  int max_size = 800;
#ifdef HOST_AARCH64
  int num_patterns = 5;
#else
  int num_patterns = 8;
#endif
  // args: buffer1, buffer2, loop count
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      fprintf(fp, ".global register_file_size_%d_%d\n", size, pattern);
      fprintf(fp, ".align 4\n");
      fprintf(fp, "register_file_size_%d_%d:\n", size, pattern);
#if defined(HOST_AMD64)
      fprintf(fp, "\tpushq %%r12\n");
      fprintf(fp, "\tpushq %%r13\n");
      fprintf(fp, "\tpushq %%r14\n");
      fprintf(fp, "\tpushq %%r15\n");
      fprintf(fp, "\tpushq %%rbx\n");

      // read two chains
      fprintf(fp, "\tmovq (%%rdi), %%rbx\n");
      fprintf(fp, "\tmovq (%%rsi), %%rcx\n");

      // loop count
      fprintf(fp, "\t1:\n");

      // first long latency load
      // NOTE: if we use long chain of sqrt on x86(both Intel and AMD), all
      // three patterns will meet a unknown limit.
      fprintf(fp, "\tmovq (%%rbx), %%rbx\n");
      // for (int j = 0; j < 10; j++) {
      //   fprintf(fp, "\tvsqrtsd %%xmm0, %%xmm0, %%xmm0\n");
      // }

      for (int j = 0; j < size; j++) {
        // instruction patterns
        // minus two: two loads also use integer rf
        if (pattern == 0 && j < size - 2) {
          // 32-bit alu: r8 to r15 to avoid dependency chain
          // use lea to avoid spamming flags register
          fprintf(fp, "\tlea (%%esp, %%r%dd), %%r%dd\n", 8 + (j % 8),
                  8 + (j % 8));
        } else if (pattern == 1 && j < size - 2) {
          // 64-bit alu: r8 to r15 to avoid dependency chain
          // use lea to avoid spamming flags register
          fprintf(fp, "\tlea (%%rsp, %%r%d), %%r%d\n", 8 + (j % 8),
                  8 + (j % 8));
        } else if (pattern == 2) {
          // flags
          fprintf(fp, "\ttest %%r15, %%r14\n");
        } else if (pattern == 3) {
          // rename
          fprintf(fp, "\tmov $0, %%r8\n");
        } else if (pattern == 4) {
          // 32-bit fp: xmm3 to xmm10
          fprintf(fp, "\tvaddss %%xmm2, %%xmm%d, %%xmm%d\n", 3 + (j % 8),
                  3 + (j % 8));
        } else if (pattern == 5) {
          // 128-bit fp: xmm3 to xmm10
          fprintf(fp, "\tvaddps %%xmm2, %%xmm2, %%xmm%d\n", 3 + (j % 8));
        } else if (pattern == 6) {
          // 256-bit fp: ymm3 to ymm10
          fprintf(fp, "\tvaddps %%ymm2, %%ymm2, %%ymm%d\n", 3 + (j % 8));
        } else if (pattern == 7) {
          // 512-bit fp: zmm3 to zmm10
          fprintf(fp, "\tvaddps %%zmm2, %%zmm2, %%zmm%d\n", 3 + (j % 8));
        }
      }

      // last long latency load
      fprintf(fp, "\tmovq (%%rcx), %%rcx\n");
      // for (int j = 0; j < 10; j++) {
      //   fprintf(fp, "\tvsqrtsd %%xmm1, %%xmm1, %%xmm1\n");
      // }

      // forbit further speculation
      fprintf(fp, "\tlfence\n");
      fprintf(fp, "\tmfence\n");

      fprintf(fp, "\tsub $1, %%rdx\n");
      fprintf(fp, "\tjne 1b\n");

      // save two chains
      fprintf(fp, "\tmovq %%rbx, (%%rdi)\n");
      fprintf(fp, "\tmovq %%rcx, (%%rsi)\n");

      // return
      fprintf(fp, "\tpopq %%rbx\n");
      fprintf(fp, "\tpopq %%r15\n");
      fprintf(fp, "\tpopq %%r14\n");
      fprintf(fp, "\tpopq %%r13\n");
      fprintf(fp, "\tpopq %%r12\n");
      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      fprintf(fp, "\t1:\n");

      // use sqrt for fp test
      bool use_sqrt = pattern != 2;
      if (use_sqrt) {
        // first chain of sqrt
        for (int j = 0; j < 10; j++) {
          fprintf(fp, "\tfsqrt d0, d0\n");
        }
      } else {
        // first chain of udiv
        for (int j = 0; j < 20; j++) {
          fprintf(fp, "\tudiv x3, x3, x3\n");
        }
      }

      for (int j = 0; j < size; j++) {
        // instruction patterns
        if (pattern == 0) {
          // 32-bit alu: x9 to x16 to avoid dependency chain
          fprintf(fp, "\tadd w%d, w%d, w8\n", 9 + (j % 8), 9 + (j % 8));
        } else if (pattern == 1) {
          // 64-bit alu: x9 to x16 to avoid dependency chain
          fprintf(fp, "\tadd x%d, x%d, x8\n", 9 + (j % 8), 9 + (j % 8));
        } else if (pattern == 2) {
          // flags
          fprintf(fp, "\tcmp x14, x15\n");
        } else if (pattern == 3) {
          // rename
          fprintf(fp, "\tmov x9, #0\n");
        } else if (pattern == 4) {
          // 32-bit fp: s16 to s23 to avoid dependency chain
          fprintf(fp, "\tfadd s%d, s%d, s15\n", 16 + (j % 8), 16 + (j % 8));
        }
      }

      if (use_sqrt) {
        // second chain of sqrt
        for (int j = 0; j < 10; j++) {
          fprintf(fp, "\tfsqrt d1, d1\n");
        }
      } else {
        // second chain of udiv
        for (int j = 0; j < 20; j++) {
          fprintf(fp, "\tudiv x4, x4, x4\n");
        }
      }

      // forbit further speculation
      fprintf(fp, "\tdsb ish\n");
      fprintf(fp, "\tisb\n");

      fprintf(fp, "\tsubs x2, x2, #1\n");
      fprintf(fp, "\tbne 1b\n");

      fprintf(fp, "\tret\n");
#endif
    }
  }

  define_gadgets_array(fp, "register_file_size_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      add_gadget(fp, "register_file_size_%d_%d", size, pattern);
    }
  }
  return 0;
}
