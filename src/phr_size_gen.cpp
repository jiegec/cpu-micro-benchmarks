#include "include/utils.h"

// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 1;
  int max_size = 256;

  // args: loop count, random array
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    // always taken or not taken for dummy branches
    fprintf(fp, ".global phr_size_%d\n", size);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "phr_size_%d:\n", size);
#ifdef HOST_AARCH64
    fprintf(fp, "\t1:\n");

    // first branch target based on random value
    fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");
    // target = first_target + x11 * 4
    // x12 = first_target
    arm64_la(fp, 12, "phr_size_%d_first_target", size);
    fprintf(fp, "\tadd x12, x12, x11, lsl 2\n");
    fprintf(fp, "\tbr x12\n");
    fprintf(fp, "\tphr_size_%d_first_target:\n", size);
    fprintf(fp, "\tnop\n");

    // the first branch is cbnz w11, 2f
    for (int i = 0; i < size - 1; i++) {
      // forward always-taken branches
      // make a long chained load
      fprintf(fp, "\tldr w11, [x1, w11, uxtw #2]\n");
      fprintf(fp, "\tcbnz x1, 2f\n");
      fprintf(fp, "\t.balign 16\n");
      fprintf(fp, "\t2:\n");
    }

    // last branch based on the same random value
    fprintf(fp, "\tldr w11, [x1, w11, uxtw #2]\n");
    fprintf(fp, "\tcbnz w11, 2f\n");
    fprintf(fp, "\t2:\n");

    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    // restore regs
    fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
    // save registers
    fprintf(fp, "\tpush %%rbx\n");
    fprintf(fp, "\tpush %%rcx\n");

    fprintf(fp, "\t1:\n");

    // first branch target based on random value
    fprintf(fp, "\tmov (%%rsi, %%rdi, 4), %%ebx\n");
    fprintf(fp, "\tlea phr_size_%d_first_target(%%rip), %%rcx\n", size);
    fprintf(fp, "\tadd %%rbx, %%rcx\n");
    fprintf(fp, "\tjmp *%%rcx\n");
    fprintf(fp, "\tphr_size_%d_first_target:\n", size);
    fprintf(fp, "\tnop\n");

    // the first branch is jnz 2f
    fprintf(fp, "\ttest %%rsi, %%rsi\n");
    for (int i = 0; i < size - 1; i++) {
      // forward always-taken branches
      // make a long chained load
      // so that if branch miss counter is missing, we can see branch
      // misprediction from timing
      fprintf(fp, "\tmov (%%rsi, %%rbx, 4), %%ebx\n");
      fprintf(fp, "\tjnz 2f\n");
      fprintf(fp, "\t2:\n");
    }

    // avoid btb entry split
    for (int i = 0; i < 256; i++) {
      fprintf(fp, "\tnop\n");
    }

    // last branch based on the same random value
    fprintf(fp, "\tmov (%%rsi, %%rbx, 4), %%ebx\n");
    fprintf(fp, "\ttest %%ebx, %%ebx\n");
    fprintf(fp, "\tjnz 2f\n");
    fprintf(fp, "\t2:\n");

    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tjnz 1b\n");

    // restore regs
    fprintf(fp, "\tpop %%rcx\n");
    fprintf(fp, "\tpop %%rbx\n");
    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "phr_size_gadgets");
  for (int size = min_size; size <= max_size; size++) {
    add_gadget(fp, "phr_size_%d", size);
  }
  return 0;
}
