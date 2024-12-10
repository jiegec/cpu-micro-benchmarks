#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
#if defined(HOST_AMD64)
  // jit only
  int min_branch_toggle = 0;
  int max_branch_toggle = 0;
  int min_dummy_branches = 0;
  int max_dummy_branches = 0;
#else
  int min_branch_toggle = 2;
#ifdef __APPLE__
  // cannot surpass page size
  int max_branch_toggle = 13;
#else
  // jit only
  int max_branch_toggle = 0;
#endif
  int min_dummy_branches = 1;
  int max_dummy_branches = PHRB_BRANCHES + 1;
#endif

  // args: loop count, random array
#if defined(HOST_AMD64)
  nasm = true;
  fprintf(fp, "section .text\n");
#elif defined(HOST_AARCH64)
  fprintf(fp, ".text\n");
#endif
  for (int branch_toggle = min_branch_toggle;
       branch_toggle <= max_branch_toggle; branch_toggle++) {
    for (int dummy_branches = min_dummy_branches;
         dummy_branches <= max_dummy_branches; dummy_branches++) {
#if defined(HOST_AMD64)
      fprintf(fp, "global phr_branch_bits_location_%d_%d\n", branch_toggle,
              dummy_branches);
      fprintf(fp, "phr_branch_bits_location_%d_%d:\n", branch_toggle,
              dummy_branches);

      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      fprintf(fp, ".global phr_branch_bits_location_%d_%d\n", branch_toggle,
              dummy_branches);
      fprintf(fp, "phr_branch_bits_location_%d_%d:\n", branch_toggle,
              dummy_branches);

      fprintf(fp, "\tphr_branch_bits_location_%d_%d_loop_begin:\n",
              branch_toggle, dummy_branches);

      // read random value
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");

      // loop to shift phr
      // NOTE: generate more loops although they contribute to branch misses due
      // to history limit. we remove the extra branch misses in
      // phr_branch_bits_location.cpp. do not generate a chain of loops: btb
      // will become a bottleneck.
      fprintf(fp, "\tmov x12, 300\n");
      fprintf(fp, "\tphr_branch_bits_location_%d_%d_dummy_target:\n",
              branch_toggle, dummy_branches);
      // x13 = dummy_target
      arm64_la(fp, 13, "phr_branch_bits_location_%d_%d_dummy_target",
               branch_toggle, dummy_branches);
      // x14 = dummy_end
      arm64_la(fp, 14, "phr_branch_bits_location_%d_%d_dummy_end",
               branch_toggle, dummy_branches);
      fprintf(fp, "\tsubs x12, x12, 1\n");
      fprintf(fp, "\tcsel x13, x13, x14, ne\n");
      fprintf(fp, "\tbr x13\n");
      fprintf(fp, "\tphr_branch_bits_location_%d_%d_dummy_end:\n",
              branch_toggle, dummy_branches);

      // two branches with opposite direction
      // only one bit differs in branch address
      // same target address
      fprintf(fp, "\t.balign %d\n", (1 << (branch_toggle + 1)));
      fprintf(fp, "\tcbz w11, phr_branch_bits_location_%d_%d_first_target\n",
              branch_toggle, dummy_branches);
      // toggle one bit in address
      fprintf(fp, "\t.rep %d\n", (((1 << branch_toggle) - 4) / 4));
      fprintf(fp, "\tnop\n");
      fprintf(fp, "\t.endr\n");
      fprintf(fp, "\tcbnz w11, phr_branch_bits_location_%d_%d_first_target\n",
              branch_toggle, dummy_branches);

      fprintf(fp, "\tphr_branch_bits_location_%d_%d_first_target:\n",
              branch_toggle, dummy_branches);

      // add dummy branches
      fprintf(fp, "\t.balign 64\n");
      for (int i = 0; i < dummy_branches; i++) {
        fprintf(fp, "\tb phr_branch_bits_location_%d_%d_dummy_target_2_%d\n",
                branch_toggle, dummy_branches, i);
        fprintf(fp, "\t.balign 64\n");
        fprintf(fp, "\tphr_branch_bits_location_%d_%d_dummy_target_2_%d:\n",
                branch_toggle, dummy_branches, i);
      }

      // second random branch
      fprintf(fp, "\tcbz w11, phr_branch_bits_location_%d_%d_second_target\n",
              branch_toggle, dummy_branches);
      fprintf(fp, "\tphr_branch_bits_location_%d_%d_second_target:\n",
              branch_toggle, dummy_branches);

      // x12 = loop_begin
      arm64_la(fp, 12, "phr_branch_bits_location_%d_%d_loop_begin",
               branch_toggle, dummy_branches);
      // x13 = end_loop
      arm64_la(fp, 13, "phr_branch_bits_location_%d_%d_end_loop", branch_toggle,
               dummy_branches);
      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tcsel x12, x12, x13, ne\n");
      fprintf(fp, "\tbr x12\n");
      fprintf(fp, "\tphr_branch_bits_location_%d_%d_end_loop:\n", branch_toggle,
              dummy_branches);

      fprintf(fp, "\tret\n");
#endif
    }
  }

  define_gadgets_array(fp, "phr_branch_bits_location_gadgets");
  for (int branch_toggle = min_branch_toggle;
       branch_toggle <= max_branch_toggle; branch_toggle++) {
    for (int dummy_branches = min_dummy_branches;
         dummy_branches <= max_dummy_branches; dummy_branches++) {
      add_gadget(fp, "phr_branch_bits_location_%d_%d", branch_toggle,
                 dummy_branches);
    }
  }
  return 0;
}
