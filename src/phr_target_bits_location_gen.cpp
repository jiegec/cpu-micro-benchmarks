#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
#if defined(HOST_AMD64)
  int min_target_toggle = 0;
  int max_target_toggle = 0;
  int min_dummy_branches = 0;
  int max_dummy_branches = 0;
#else
  int min_target_toggle = 2;
#ifdef __APPLE__
  // cannot surpass page size
  int max_target_toggle = 13;
#else
  int max_target_toggle = 14;
#endif
  int min_dummy_branches = PHR_BRANCHES - 35;
  int max_dummy_branches = PHR_BRANCHES + 0;
#endif

  // args: loop count, random array
#if defined(HOST_AMD64)
  nasm = true;
  fprintf(fp, "section .text\n");
#else
  fprintf(fp, ".text\n");
#endif
  for (int target_toggle = min_target_toggle;
       target_toggle <= max_target_toggle; target_toggle++) {
    for (int dummy_branches = min_dummy_branches;
         dummy_branches <= max_dummy_branches; dummy_branches++) {
#if defined(HOST_AMD64)
      fprintf(fp, "global phr_target_bits_location_%d_%d\n", target_toggle,
              dummy_branches);
      fprintf(fp, "phr_target_bits_location_%d_%d:\n", target_toggle,
              dummy_branches);

      // jit only
      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      fprintf(fp, ".global phr_target_bits_location_%d_%d\n", target_toggle,
              dummy_branches);
      fprintf(fp, "phr_target_bits_location_%d_%d:\n", target_toggle,
              dummy_branches);

      fprintf(fp, "\tphr_target_bits_location_%d_%d_loop_begin:\n",
              target_toggle, dummy_branches);

      // read random value
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");

      // loop to shift phr
      // NOTE: generate more loops although they contribute to branch misses due
      // to history limit. we remove the extra branch misses in
      // phr_target_bits_location.cpp. do not generate a chain of loops: btb
      // will become a bottleneck.
      fprintf(fp, "\tmov x12, 200\n");
      fprintf(fp, "\tphr_target_bits_location_%d_%d_dummy_target:\n",
              target_toggle, dummy_branches);
      // x13 = dummy_target
      arm64_la(fp, 13, "phr_target_bits_location_%d_%d_dummy_target",
               target_toggle, dummy_branches);
      // x14 = dummy_target
      arm64_la(fp, 14, "phr_target_bits_location_%d_%d_dummy_end",
               target_toggle, dummy_branches);
      fprintf(fp, "\tsubs x12, x12, 1\n");
      fprintf(fp, "\tcsel x13, x13, x14, ne\n");
      fprintf(fp, "\tbr x13\n");
      fprintf(fp, "\tphr_target_bits_location_%d_%d_dummy_end:\n",
              target_toggle, dummy_branches);

      // indirect branches with conditonal target address
      // only one bit differs in target address
      // x12 = first_target_1
      arm64_la(fp, 12, "phr_target_bits_location_%d_%d_first_target_1",
               target_toggle, dummy_branches);
      // x13 = first_target_2
      arm64_la(fp, 13, "phr_target_bits_location_%d_%d_first_target_2",
               target_toggle, dummy_branches);
      fprintf(fp, "\tcmp w11, 0\n");
      fprintf(fp, "\tcsel x12, x12, x13, eq\n");
      fprintf(fp, "\tbr x12\n");

      fprintf(fp, "\t.balign %d\n", (1 << (target_toggle + 1)));
      fprintf(fp, "\tphr_target_bits_location_%d_%d_first_target_1:\n",
              target_toggle, dummy_branches);

#if defined(APPLE_FIRESTORM) || defined(QUALCOMM_ORYON)
      bool skip_nops = target_toggle >= 6;
#else
      bool skip_nops = false;
#endif
      // skip many many NOPs by branching
      // the two branches has same target address
      // branch address only differs in one bit
      // and we know these branch bits are not involved in PHR computation
      if (skip_nops) {
        // x13 = target_end
        arm64_la(fp, 13, "phr_target_bits_location_%d_%d_target_end",
                 target_toggle, dummy_branches);
        fprintf(fp, "\tbr x13\n");
      } else {
        fprintf(fp, "\tnop\n");
      }
      fprintf(fp, "\t.balign %d\n", (1 << (target_toggle)));
      fprintf(fp, "\tphr_target_bits_location_%d_%d_first_target_2:\n",
              target_toggle, dummy_branches);
      if (skip_nops) {
        arm64_la(fp, 13, "phr_target_bits_location_%d_%d_target_end",
                 target_toggle, dummy_branches);
        fprintf(fp, "\tbr x13\n");
      }
      fprintf(fp, "\tphr_target_bits_location_%d_%d_target_end:\n",
              target_toggle, dummy_branches);

      // add dummy branches
      // skip one branch if we have br above
      fprintf(fp, "\t.balign 64\n");
      for (int i = 0; i < dummy_branches - (skip_nops ? 1 : 0); i++) {
        fprintf(fp, "\tb phr_target_bits_location_%d_%d_dummy_target_2_%d\n",
                target_toggle, dummy_branches, i);
        fprintf(fp, "\t.balign 64\n");
        fprintf(fp, "\tphr_target_bits_location_%d_%d_dummy_target_2_%d:\n",
                target_toggle, dummy_branches, i);
      }

      // second random branch
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\tcbz w11, phr_target_bits_location_%d_%d_second_target\n",
              target_toggle, dummy_branches);
      fprintf(fp, "\tphr_target_bits_location_%d_%d_second_target:\n",
              target_toggle, dummy_branches);

      // x12 = loop_begin
      arm64_la(fp, 12, "phr_target_bits_location_%d_%d_loop_begin",
               target_toggle, dummy_branches);
      // x13 = loop_begin
      arm64_la(fp, 13, "phr_target_bits_location_%d_%d_end_loop", target_toggle,
               dummy_branches);
      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tcsel x12, x12, x13, ne\n");
      fprintf(fp, "\tbr x12\n");
      fprintf(fp, "\tphr_target_bits_location_%d_%d_end_loop:\n", target_toggle,
              dummy_branches);

      fprintf(fp, "\tret\n");
#else
      fprintf(fp, ".global phr_target_bits_location_%d_%d\n", target_toggle,
              dummy_branches);
      fprintf(fp, "phr_target_bits_location_%d_%d:\n", target_toggle,
              dummy_branches);
      fprintf(fp, "\tret\n");
#endif
    }
  }

  define_gadgets_array(fp, "phr_target_bits_location_gadgets");
  for (int target_toggle = min_target_toggle;
       target_toggle <= max_target_toggle; target_toggle++) {
    for (int dummy_branches = min_dummy_branches;
         dummy_branches <= max_dummy_branches; dummy_branches++) {
      add_gadget(fp, "phr_target_bits_location_%d_%d", target_toggle,
                 dummy_branches);
    }
  }
  return 0;
}
