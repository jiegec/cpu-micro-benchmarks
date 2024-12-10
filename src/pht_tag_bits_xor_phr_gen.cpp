#include "include/utils.h"

// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf
// https://arxiv.org/pdf/2411.13900
// find tag conflict in PHR vs PHR
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_first_phr_bit = 0;
  int max_first_phr_bit = 35;
  int min_dummy_branches = 0;
  int max_dummy_branches = PHR_BRANCHES;
  // check PHR[first_phr_bit] vs PHR[dummy_branches] vs PHR[third_phr_bit]
  // PHR[first_phr_bit]=PHR[dummy_branches]
  // k = PHR[first_phr_bit] xor PHR[third_phr_bit]
  int third_phr_bit = PHR_BRANCHES - 1;
  char gadget_name[128];

  // args: loop count, random array
  fprintf(fp, ".text\n");
  for (int inject_target = 0; inject_target <= 1; inject_target++) {
    for (int first_phr_bit = min_first_phr_bit;
         first_phr_bit <= max_first_phr_bit; first_phr_bit++) {
      for (int dummy_branches = min_dummy_branches;
           dummy_branches <= max_dummy_branches; dummy_branches++) {
        sprintf(gadget_name, "pht_tag_bits_xor_phr_%d_%d_%d", inject_target,
                first_phr_bit, dummy_branches);
        fprintf(fp, ".global %s\n", gadget_name);
        fprintf(fp, "%s:\n", gadget_name);
#ifdef HOST_AARCH64
        fprintf(fp, "\t%s_loop_begin:\n", gadget_name);

        // read random value
        fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");

        // loop to shift phr
        for (int i = 0; i < 200; i++) {
          fprintf(fp, "\tb %s_dummy_target1_%d\n", gadget_name, i);
          fprintf(fp, "\t%s_dummy_target1_%d:\n", gadget_name, i);
        }

        // inject w11 to phr LSB using T[2]
        arm64_la(fp, 12, "%s_first_target_1", gadget_name);
        arm64_la(fp, 13, "%s_first_target_2", gadget_name);
        // branch based on w11[0]
        fprintf(fp, "\tand w14, w11, 1\n");
        fprintf(fp, "\tcmp w14, 0\n");
        fprintf(fp, "\tcsel x12, x12, x13, eq\n");
        fprintf(fp, "\tbr x12\n");
        // T[2] differs
        fprintf(fp, "\t.balign 8\n");
        fprintf(fp, "\t%s_first_target_1:\n", gadget_name);
        fprintf(fp, "\tnop\n");
        fprintf(fp, "\t%s_first_target_2:\n", gadget_name);

        // loop to shift phr by third_phr_bit - dummy_branches - 1 times
        for (int i = 0; i < third_phr_bit - dummy_branches - 1; i++) {
          fprintf(fp, "\tb %s_dummy0_target0_%d\n", gadget_name, i);
          fprintf(fp, "\t%s_dummy0_target0_%d:\n", gadget_name, i);
        }

        // inject w11 to phr LSB
        arm64_la(fp, 12, "%s_mid_target_1", gadget_name);
        arm64_la(fp, 13, "%s_mid_target_2", gadget_name);
        // branch based on w11[1]
        fprintf(fp, "\tand w14, w11, 2\n");
        fprintf(fp, "\tcmp w14, 0\n");
        fprintf(fp, "\tcsel x12, x12, x13, eq\n");
        fprintf(fp, "\tbr x12\n");
        // T[2] differs
        fprintf(fp, "\t.balign 8\n");
        fprintf(fp, "\t%s_mid_target_1:\n", gadget_name);
        fprintf(fp, "\tnop\n");
        fprintf(fp, "\t%s_mid_target_2:\n", gadget_name);

        // loop to shift phr by dummybranches - first_phr_bit -
        // (inject_target ? 1 : 3) times
        // so that the bit above will locate at PHR[dummybranches]
        for (int i = 0;
             i < dummy_branches - first_phr_bit - (inject_target ? 1 : 3);
             i++) {
          fprintf(fp, "\tb %s_dummy2_target2_%d\n", gadget_name, i);
          fprintf(fp, "\t%s_dummy2_target2_%d:\n", gadget_name, i);
        }

        if (inject_target) {
          // inject w11 to phr LSB again
          arm64_la(fp, 12, "%s_second_target_1", gadget_name);
          arm64_la(fp, 13, "%s_second_target_2", gadget_name);
          // branch based on w11[1]
          fprintf(fp, "\tand w14, w11, 2\n");
          fprintf(fp, "\tcmp w14, 0\n");
          fprintf(fp, "\tcsel x12, x12, x13, eq\n");
          fprintf(fp, "\tbr x12\n");
          // T[2] differs
          fprintf(fp, "\t.balign 8\n");
          fprintf(fp, "\t%s_second_target_1:\n", gadget_name);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t%s_second_target_2:\n", gadget_name);
        } else {
          // inject w11 to phr LSB using B[2]
          // 1. same B, T[2] differs
          // 2. B[2] differs, T[3] differs
          // 3. B[2] & B[3] differs, same T

          // 1. same B, T[2] differs
          arm64_la(fp, 12, "%s_second_target_1", gadget_name);
          arm64_la(fp, 13, "%s_second_target_2", gadget_name);
          // branch based on w11[1]
          fprintf(fp, "\tand w14, w11, 2\n");
          fprintf(fp, "\tcmp w14, 0\n");
          fprintf(fp, "\tcsel x12, x12, x13, eq\n");
          fprintf(fp, "\tbr x12\n");
          // T[2] differs
          fprintf(fp, "\t.balign 8\n");
          fprintf(fp, "\t%s_second_target_1:\n", gadget_name);
          fprintf(fp, "\tb %s_second_target_3\n", gadget_name);
          fprintf(fp, "\t%s_second_target_2:\n", gadget_name);
          fprintf(fp, "\tb %s_second_target_4\n", gadget_name);

          // 2. B[2] differs, T[3] differs
          fprintf(fp, "\t.balign 16\n");
          fprintf(fp, "\t%s_second_target_3:\n", gadget_name);
          fprintf(fp, "\tb %s_second_target_5\n", gadget_name);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t%s_second_target_4:\n", gadget_name);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\tb %s_second_target_5\n", gadget_name);

          // 3. B[2] & B[3] differs, same T
          fprintf(fp, "\t%s_second_target_5:\n", gadget_name);
        }

        // loop to shift phr by first_phr_bit times
        for (int i = 0; i < first_phr_bit; i++) {
          fprintf(fp, "\tb %s_dummy_target2_%d\n", gadget_name, i);
          fprintf(fp, "\t%s_dummy_target2_%d:\n", gadget_name, i);
        }

        // conditional branch based on x11[1] ^ x11[0]
        fprintf(fp, "\tand w14, w11, 2\n");
        fprintf(fp, "\tlsr w14, w14, 1\n");
        fprintf(fp, "\tand w11, w11, 1\n");
        fprintf(fp, "\teor w11, w11, w14\n");
        fprintf(fp, "\tcbnz x11, %s_first_target\n", gadget_name);
        fprintf(fp, "\t%s_first_target:\n", gadget_name);

        // loop end
        arm64_la(fp, 12, "%s_loop_begin", gadget_name);
        arm64_la(fp, 13, "%s_end_loop", gadget_name);
        fprintf(fp, "\tsubs x0, x0, #1\n");
        fprintf(fp, "\tcsel x12, x12, x13, ne\n");
        fprintf(fp, "\tbr x12\n");

        fprintf(fp, "\t%s_end_loop:\n", gadget_name);

        fprintf(fp, "\tret\n");
#endif
      }
    }
  }

  define_gadgets_array(fp, "pht_tag_bits_xor_phr_gadgets");
  for (int inject_target = 0; inject_target <= 1; inject_target++) {
    for (int first_phr_bit = min_first_phr_bit;
         first_phr_bit <= max_first_phr_bit; first_phr_bit++) {
      for (int dummy_branches = min_dummy_branches;
           dummy_branches <= max_dummy_branches; dummy_branches++) {
        add_gadget(fp, "pht_tag_bits_xor_phr_%d_%d_%d", inject_target,
                   first_phr_bit, dummy_branches);
      }
    }
  }
  return 0;
}
