#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_branch_align = 2;
#ifdef __APPLE__
  // cannot surpass page size
  int max_branch_align = 13;
#else
  int max_branch_align = 19;
#endif

  // args: loop count, random array
  fprintf(fp, ".text\n");
  for (int branch_align = min_branch_align; branch_align <= max_branch_align;
       branch_align++) {
    fprintf(fp, ".global pht_index_tag_bits_%d\n", branch_align);
    fprintf(fp, "pht_index_tag_bits_%d:\n", branch_align);
#ifdef HOST_AARCH64
    fprintf(fp, "\tpht_index_tag_bits_%d_loop_begin:\n", branch_align);

    // read random value
    fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");

    // loop to shift phr
    for (int i = 0; i < 200; i++) {
      fprintf(fp, "\tb pht_index_tag_bits_%d_dummy_target1_%d\n", branch_align,
              i);
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\tpht_index_tag_bits_%d_dummy_target1_%d:\n", branch_align,
              i);
    }

    // inject w11 to phr LSB
    arm64_la(fp, 12, "pht_index_tag_bits_%d_first_target_1", branch_align);
    arm64_la(fp, 13, "pht_index_tag_bits_%d_first_target_2", branch_align);
    fprintf(fp, "\tcmp w11, 0\n");
    fprintf(fp, "\tcsel x12, x12, x13, eq\n");
    fprintf(fp, "\tbr x12\n");

    // T[t_bit] differs
    int t_bit = 2;
    fprintf(fp, "\t.balign (1<<%d)\n", t_bit + 1);
    fprintf(fp, "\tpht_index_tag_bits_%d_first_target_1:\n", branch_align);
    for (int i = 0; i < (1 << t_bit) / 4; i++) {
      fprintf(fp, "\tnop\n");
    }
    fprintf(fp, "\tpht_index_tag_bits_%d_first_target_2:\n", branch_align);

    // loop to shift phr by PHR_BRANCHES-1 times
    for (int i = 0; i < PHR_BRANCHES - 1; i++) {
      fprintf(fp, "\tb pht_index_tag_bits_%d_dummy_target2_%d\n", branch_align,
              i);
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\tpht_index_tag_bits_%d_dummy_target2_%d:\n", branch_align,
              i);
    }

    // address only differs in PC[branch_align]
    fprintf(fp, "\t.balign %d\n", (1 << (branch_align + 1)));
    fprintf(fp, "\tcbnz x11, pht_index_tag_bits_%d_branch_end\n", branch_align);
    fprintf(fp, "\t.balign %d\n", (1 << branch_align));
    fprintf(fp, "\tcbz x11, pht_index_tag_bits_%d_branch_end\n", branch_align);
    fprintf(fp, "\tpht_index_tag_bits_%d_branch_end:\n", branch_align);

    // loop end
    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tcbz x0, pht_index_tag_bits_%d_end_loop\n", branch_align);
    arm64_la(fp, 13, "pht_index_tag_bits_%d_loop_begin", branch_align);
    fprintf(fp, "\tbr x13\n");
    fprintf(fp, "\tpht_index_tag_bits_%d_end_loop:\n", branch_align);

    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "pht_index_tag_bits_gadgets");
  for (int branch_align = min_branch_align; branch_align <= max_branch_align;
       branch_align++) {
    add_gadget(fp, "pht_index_tag_bits_%d", branch_align);
  }
  return 0;
}
