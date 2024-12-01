#include "include/utils.h"

// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/tests/ReturnStackTest.cs
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 1;
  int max_size = 128;

  // args: loop count
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    // entry
    fprintf(fp, ".global ras_size_%d\n", size);
    fprintf(fp, ".balign 64\n");
    fprintf(fp, "ras_size_%d:\n", size);
#ifdef HOST_AARCH64
    // save lr
    fprintf(fp, "\tsub sp, sp, #0x20\n");
    fprintf(fp, "\tstp x29, x30, [sp, #0x10]\n");

    fprintf(fp, "\t1:\n");
    // call function
    fprintf(fp, "\tbl ras_func_%d\n", size - 1);
    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    // restore lr
    fprintf(fp, "\tldp x29, x30, [sp, #0x10]\n");
    fprintf(fp, "\tadd sp, sp, #0x20\n");
    fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
    fprintf(fp, "\t1:\n");
    // call function
    fprintf(fp, "\tcall ras_func_%d\n", size - 1);
    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tjne 1b\n");
    fprintf(fp, "\tret\n");
#endif

    // inner function
    fprintf(fp, ".global ras_func_%d\n", size);
    fprintf(fp, ".balign 64\n");
    fprintf(fp, "ras_func_%d:\n", size);

    // TODO: if we don't want BTB to predict target address for ret
    // we can use two bl, and alternate between the two using x0

#ifdef HOST_AARCH64
    // save lr
    fprintf(fp, "\tsub sp, sp, #0x20\n");
    fprintf(fp, "\tstp x29, x30, [sp, #0x10]\n");

    // call lower function
    fprintf(fp, "\tbl ras_func_%d\n", size - 1);

    // restore lr
    fprintf(fp, "\tldp x29, x30, [sp, #0x10]\n");
    fprintf(fp, "\tadd sp, sp, #0x20\n");
    fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
    fprintf(fp, "\tcall ras_func_%d\n", size - 1);
    fprintf(fp, "\tret\n");
#endif
  }

  // recursion base
  fprintf(fp, ".global ras_func_%d\n", 0);
  fprintf(fp, ".balign 32\n");
  fprintf(fp, "ras_func_%d:\n", 0);
  fprintf(fp, "\tret\n");

  define_gadgets_array(fp, "ras_size_gadgets");
  for (int size = min_size; size <= max_size; size++) {
    add_gadget(fp, "ras_size_%d", size);
  }
  return 0;
}
