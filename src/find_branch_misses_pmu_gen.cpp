#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int num_patterns = 3;
  // args: loop count
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    fprintf(fp, ".global find_branch_misses_pmu_%d\n", pattern);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "find_branch_misses_pmu_%d:\n", pattern);

#if defined(HOST_AARCH64)
    fprintf(fp, "\t1:\n");

    // flush phr
    for (int i = 0; i < 200; i++) {
      fprintf(fp, "\tb 2f\n");
      fprintf(fp, "\t.balign 16\n");
      fprintf(fp, "\t2:\n");
    }

    if (pattern == 0) {
      // conditional branch miss
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");
      fprintf(fp, "\tcbnz w11, 2f\n");
      fprintf(fp, "\tadd x12, x12, 1\n");
      fprintf(fp, "\t2:\n");
    } else if (pattern == 1) {
      // indirect branch misses
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");
      arm64_la(fp, 12, "first_target");
      fprintf(fp, "\tadd x12, x12, x11, lsl 2\n");
      fprintf(fp, "\tbr x12\n");
      fprintf(fp, "\tfirst_target:\n");
      fprintf(fp, "\tnop\n");
    } else if (pattern == 2) {
      // conditional + indirect branch miss

      // conditional branch miss
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");
      fprintf(fp, "\tcbnz w11, 2f\n");
      fprintf(fp, "\tadd x12, x12, 1\n");
      fprintf(fp, "\t2:\n");

      // flush phr again
      for (int i = 0; i < 500; i++) {
        fprintf(fp, "\tb 2f\n");
        fprintf(fp, "\t.balign 16\n");
        fprintf(fp, "\t2:\n");
      }

      // indirect branch misses
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");
      arm64_la(fp, 12, "second_target");
      fprintf(fp, "\tadd x12, x12, x11, lsl 2\n");
      fprintf(fp, "\tbr x12\n");
      fprintf(fp, "\tsecond_target:\n");
      fprintf(fp, "\tnop\n");
    }

    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
    // save registers
    fprintf(fp, "\tpush %%rbx\n");
    fprintf(fp, "\tpush %%rcx\n");

    fprintf(fp, "\t1:\n");

    // flush phr
    for (int i = 0; i < 200; i++) {
      fprintf(fp, "\tjmp 2f\n");
      fprintf(fp, "\t2:\n");
    }

    if (pattern == 0) {
      // conditional branch miss
      fprintf(fp, "\tmov (%%rsi, %%rdi, 4), %%ebx\n");
      fprintf(fp, "\ttest %%ebx, %%ebx\n");
      fprintf(fp, "\tjnz 2f\n");
      fprintf(fp, "\t2:\n");
    } else if (pattern == 1) {
      // indirect branch misses
      fprintf(fp, "\tmov (%%rsi, %%rdi, 4), %%ebx\n");
      fprintf(fp, "\tlea first_target(%%rip), %%rdx\n");
      fprintf(fp, "\tadd %%rbx, %%rdx\n");
      fprintf(fp, "\tjmp *%%rdx\n");
      fprintf(fp, "\tfirst_target:\n");
      fprintf(fp, "\tnop\n");
    } else if (pattern == 2) {
      // conditional + indirect branch miss

      // conditional branch miss
      fprintf(fp, "\tmov (%%rsi, %%rdi, 4), %%ebx\n");
      fprintf(fp, "\ttest %%ebx, %%ebx\n");
      fprintf(fp, "\tjnz 2f\n");
      fprintf(fp, "\t2:\n");

      // flush phr again
      for (int i = 0; i < 200; i++) {
        fprintf(fp, "\tjmp 2f\n");
        fprintf(fp, "\t2:\n");
      }

      // indirect branch misses
      fprintf(fp, "\tmov (%%rsi, %%rdi, 4), %%ebx\n");
      fprintf(fp, "\tlea second_target(%%rip), %%rdx\n");
      fprintf(fp, "\tadd %%rbx, %%rdx\n");
      fprintf(fp, "\tjmp *%%rdx\n");
      fprintf(fp, "\tsecond_target:\n");
      fprintf(fp, "\tnop\n");
    }

    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tjnz 1b\n");

    // restore regs
    fprintf(fp, "\tpop %%rcx\n");
    fprintf(fp, "\tpop %%rbx\n");
    fprintf(fp, "\tret\n");
#elif defined(__loongarch__)
    fprintf(fp, "\t1:\n");

    // flush phr
    for (int i = 0; i < 200; i++) {
      fprintf(fp, "\tb 2f\n");
      fprintf(fp, "\t2:\n");
    }

    if (pattern == 0) {
      // conditional branch miss
      fprintf(fp, "\tslli.d $t0, $a0, 2\n");
      fprintf(fp, "\tldx.w $t0, $a1, $t0\n");
      fprintf(fp, "\tbeqz $t0, 2f\n");
      fprintf(fp, "\t2:\n");
    } else if (pattern == 1) {
      // indirect branch misses
      fprintf(fp, "\tslli.d $t0, $a0, 2\n");
      fprintf(fp, "\tldx.w $t0, $a1, $t0\n");
      fprintf(fp, "\tslli.d $t0, $t0, 2\n");
      fprintf(fp, "\tla.global $t1, 2f\n");
      fprintf(fp, "\tadd.d $t0, $t1, $t0\n");
      fprintf(fp, "\tjirl $zero, $t0, 0\n");
      fprintf(fp, "\t2:\n");
      fprintf(fp, "\tnop\n");
    } else if (pattern == 2) {
      // conditional + indirect branch miss

      // conditional branch miss
      fprintf(fp, "\tslli.d $t0, $a0, 2\n");
      fprintf(fp, "\tldx.w $t0, $a1, $t0\n");
      fprintf(fp, "\tbeqz $t0, 2f\n");
      fprintf(fp, "\t2:\n");

      // flush phr again
      for (int i = 0; i < 200; i++) {
        fprintf(fp, "\tb 2f\n");
        fprintf(fp, "\t2:\n");
      }

      // indirect branch misses
      fprintf(fp, "\tslli.d $t0, $a0, 2\n");
      fprintf(fp, "\tldx.w $t0, $a1, $t0\n");
      fprintf(fp, "\tslli.d $t0, $t0, 2\n");
      fprintf(fp, "\tla.global $t1, 2f\n");
      fprintf(fp, "\tadd.d $t0, $t1, $t0\n");
      fprintf(fp, "\tjirl $zero, $t0, 0\n");
      fprintf(fp, "\t2:\n");
      fprintf(fp, "\tnop\n");
    }

    fprintf(fp, "\taddi.d $a0, $a0, -1\n");
    fprintf(fp, "\tbnez $a0, 1b\n");

    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "find_branch_misses_pmu_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    add_gadget(fp, "find_branch_misses_pmu_%d", pattern);
  }
  return 0;
}
