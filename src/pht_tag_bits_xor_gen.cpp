#include "include/utils.h"

// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf
// https://arxiv.org/pdf/2411.13900
// find tag conflict in PHR vs PC
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_branch_align = 3;
#ifdef __APPLE__
  int max_branch_align = 13;
#else
  int max_branch_align = 18;
#endif
  int min_dummy_branches = 0;
  int max_dummy_branches = PHR_BRANCHES + 5;
  char gadget_name[128];

  // args: loop count, random array
  fprintf(fp, ".text\n");
  for (int inject_target = 0; inject_target <= 1; inject_target++) {
    for (int branch_align = min_branch_align; branch_align <= max_branch_align;
         branch_align++) {
      for (int dummy_branches = min_dummy_branches;
           dummy_branches <= max_dummy_branches; dummy_branches++) {
        sprintf(gadget_name, "pht_tag_bits_xor_%d_%d_%d", inject_target,
                branch_align, dummy_branches);
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

        if (inject_target) {
          // inject w11 to phr LSB using T[2]
          arm64_la(fp, 12, "%s_first_target_1", gadget_name);
          arm64_la(fp, 13, "%s_first_target_2", gadget_name);
          fprintf(fp, "\tcmp w11, 0\n");
          fprintf(fp, "\tcsel x12, x12, x13, eq\n");
          fprintf(fp, "\tbr x12\n");
          // T[2] differs
          fprintf(fp, "\t.balign 8\n");
          fprintf(fp, "\t%s_first_target_1:\n", gadget_name);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t%s_first_target_2:\n", gadget_name);
        } else {
          // inject w11 to phr LSB using B[2]
          // 1. same B, T[2] differs
          // 2. B[2] differs, T[3] differs
          // 3. B[2] & B[3] differs, same T

          // 1. same B, T[2] differs
          arm64_la(fp, 12, "%s_first_target_1", gadget_name);
          arm64_la(fp, 13, "%s_first_target_2", gadget_name);
          fprintf(fp, "\tcmp w11, 0\n");
          fprintf(fp, "\tcsel x12, x12, x13, eq\n");
          // T[2] differs
          fprintf(fp, "\tbr x12\n");
          fprintf(fp, "\t.balign 8\n");
          fprintf(fp, "\t%s_first_target_1:\n", gadget_name);
          fprintf(fp, "\tb %s_first_target_3\n", gadget_name);
          fprintf(fp, "\t%s_first_target_2:\n", gadget_name);
          fprintf(fp, "\tb %s_first_target_4\n", gadget_name);

          // 2. B[2] differs, T[3] differs
          fprintf(fp, "\t.balign 16\n");
          fprintf(fp, "\t%s_first_target_3:\n", gadget_name);
          fprintf(fp, "\tb %s_first_target_5\n", gadget_name);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t%s_first_target_4:\n", gadget_name);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\tb %s_first_target_5\n", gadget_name);

          // 3. B[2] & B[3] differs, same T
          fprintf(fp, "\t%s_first_target_5:\n", gadget_name);
        }

        // loop to shift phr by dummybranches times
        // remove extra branches below
        for (int i = 0; i < dummy_branches - (branch_align >= 7 ? 2 : 3); i++) {
          fprintf(fp, "\tb %s_dummy_target2_%d\n", gadget_name, i);
          fprintf(fp, "\t%s_dummy_target2_%d:\n", gadget_name, i);
        }

        // strategy #1:
        // two branches:
        // 1. B same, T[6] differs
        // 2. B same (B[6] not counted), T[7] differs
        // result: PHR same, PC[7] differs
        // applicable for PC[7+]
        if (branch_align >= 7) {
          // 1. B same, T[6] differs
          arm64_la(fp, 12, "%s_target1", gadget_name);
          arm64_la(fp, 13, "%s_target2", gadget_name);
          fprintf(fp, "\tand x14, x0, 0x1\n");
          fprintf(fp, "\tcmp x14, 0\n");
          fprintf(fp, "\tcsel x12, x12, x13, ne\n");
          fprintf(fp, "\tbr x12\n");

          // 2. B same (B[6] not counted), T[7] differs
          fprintf(fp, "\t.balign %d\n", (1 << branch_align));
          fprintf(fp, "\t%s_target1:\n", gadget_name);
          fprintf(fp, "\tb %s_target3\n", gadget_name);
          fprintf(fp, "\t.balign %d\n", (1 << (branch_align - 1)));
          fprintf(fp, "\t%s_target2:\n", gadget_name);
          fprintf(fp, "\tb %s_target4\n", gadget_name);

          // result: PHR same, PC[7] differs
          // same direction, see if they conflict
          fprintf(fp, "\t.balign %d\n", (1 << (branch_align + 1)));
          fprintf(fp, "\t%s_target3:\n", gadget_name);
          fprintf(fp, "\tcbnz x11, %s_first_target\n", gadget_name);
          fprintf(fp, "\t%s_first_target:\n", gadget_name);
          fprintf(fp, "\tb %s_branch_end\n", gadget_name);

          fprintf(fp, "\t.balign %d\n", (1 << branch_align));
          fprintf(fp, "\t%s_target4:\n", gadget_name);
          fprintf(fp, "\tcbnz x11, %s_second_target\n", gadget_name);
          fprintf(fp, "\t%s_second_target:\n", gadget_name);
          fprintf(fp, "\tb %s_branch_end\n", gadget_name);
        }

        // strategy #2:
        // three branches:
        // 1. B same, T[2] differs
        // 2. B[2] differs, T[2] & T[3] differs
        // 3. B[3] differs, T[3] differs
        // result: PHR same, PC[3] differs
        // applicable for PC[3:5]
        if (branch_align <= 5) {
          // 1. B same, T[2] differs
          arm64_la(fp, 12, "%s_target1", gadget_name);
          arm64_la(fp, 13, "%s_target2", gadget_name);
          fprintf(fp, "\tand x14, x0, 0x1\n");
          fprintf(fp, "\tcmp x14, 0\n");
          fprintf(fp, "\tcsel x12, x12, x13, ne\n");
          fprintf(fp, "\tbr x12\n");

          // 2. B[2] differs, T[2] & T[3] differs
          fprintf(fp, "\t.balign %d\n", (1 << branch_align));
          fprintf(fp, "\t%s_target1:\n", gadget_name);
          fprintf(fp, "\tb %s_target3\n", gadget_name);
          fprintf(fp, "\t.balign %d\n", (1 << (branch_align - 1)));
          fprintf(fp, "\t%s_target2:\n", gadget_name);
          fprintf(fp, "\tb %s_target4\n", gadget_name);

          // 3. B[3] differs, T[3] differs
          fprintf(fp, "\t.balign %d\n", (1 << (branch_align + 1)));
          fprintf(fp, "\t%s_target3:\n", gadget_name);
          fprintf(fp, "\t.rept %d\n", ((1 << (branch_align - 1))) / 4);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t.endr\n");
          fprintf(fp, "\tb %s_target5\n", gadget_name);
          fprintf(fp, "\t.balign %d\n", (1 << branch_align));
          fprintf(fp, "\t.rept %d\n", ((1 << (branch_align - 1))) / 4);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t.endr\n");
          fprintf(fp, "\t%s_target4:\n", gadget_name);
          fprintf(fp, "\tb %s_target6\n", gadget_name);

          // result: PHR same, PC[3] differs
          // same direction, see if they conflict
          fprintf(fp, "\t.balign %d\n", (1 << (branch_align + 1)));
          fprintf(fp, "\t%s_target5:\n", gadget_name);
          fprintf(fp, "\tcbnz x11, %s_first_target\n", gadget_name);
          fprintf(fp, "\t%s_first_target:\n", gadget_name);
          fprintf(fp, "\tb %s_branch_end\n", gadget_name);

          fprintf(fp, "\t.balign %d\n", (1 << branch_align));
          fprintf(fp, "\t%s_target6:\n", gadget_name);
          fprintf(fp, "\tcbnz x11, %s_second_target\n", gadget_name);
          fprintf(fp, "\t%s_second_target:\n", gadget_name);
          fprintf(fp, "\tb %s_branch_end\n", gadget_name);
        }

        // strategy #3:
        // three branches:
        // 1. B same, T[3] differs
        // 2. B[3] differs, T[5] & T[4] differs
        // 3. B[4] differs, T[6] differs
        // result: PHR same, PC[6] differs
        // applicable for PC[6]
        if (branch_align == 6) {
          // 1. B same, T[3] differs
          arm64_la(fp, 12, "%s_target1", gadget_name);
          arm64_la(fp, 13, "%s_target2", gadget_name);
          fprintf(fp, "\tand x14, x0, 0x1\n");
          fprintf(fp, "\tcmp x14, 0\n");
          fprintf(fp, "\tcsel x12, x12, x13, ne\n");
          fprintf(fp, "\tbr x12\n");

          // 2. B[3] differs, T[5] & T[4] differs
          fprintf(fp, "\t.balign %d\n", (1 << 4));
          fprintf(fp, "\t%s_target1:\n", gadget_name);
          fprintf(fp, "\tb %s_target3\n", gadget_name);
          fprintf(fp, "\t.balign %d\n", (1 << 3));
          fprintf(fp, "\t%s_target2:\n", gadget_name);
          fprintf(fp, "\tb %s_target4\n", gadget_name);

          // 3. B[4] differs, T[6] differs
          fprintf(fp, "\t.balign %d\n", (1 << 6));
          fprintf(fp, "\t%s_target3:\n", gadget_name);
          fprintf(fp, "\t.rept %d\n", ((1 << 5)) / 4);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t.endr\n");
          fprintf(fp, "\tb %s_target5\n", gadget_name);
          fprintf(fp, "\t.rept %d\n", ((1 << 5) - (1 << 4) - 4) / 4);
          fprintf(fp, "\tnop\n");
          fprintf(fp, "\t.endr\n");
          fprintf(fp, "\t%s_target4:\n", gadget_name);
          fprintf(fp, "\tb %s_target6\n", gadget_name);

          // result: PHR same, PC[6] differs
          // same direction, see if they conflict
          fprintf(fp, "\t.balign %d\n", (1 << 7));
          fprintf(fp, "\t%s_target5:\n", gadget_name);
          fprintf(fp, "\tcbnz x11, %s_first_target\n", gadget_name);
          fprintf(fp, "\t%s_first_target:\n", gadget_name);
          fprintf(fp, "\tb %s_branch_end\n", gadget_name);

          fprintf(fp, "\t.balign %d\n", (1 << 6));
          fprintf(fp, "\t%s_target6:\n", gadget_name);
          fprintf(fp, "\tcbnz x11, %s_second_target\n", gadget_name);
          fprintf(fp, "\t%s_second_target:\n", gadget_name);
          fprintf(fp, "\tb %s_branch_end\n", gadget_name);
        }

        fprintf(fp, "\t%s_branch_end:\n", gadget_name);

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

  define_gadgets_array(fp, "pht_tag_bits_xor_gadgets");
  for (int inject_target = 0; inject_target <= 1; inject_target++) {
    for (int branch_align = min_branch_align; branch_align <= max_branch_align;
         branch_align++) {
      for (int dummy_branches = min_dummy_branches;
           dummy_branches <= max_dummy_branches; dummy_branches++) {
        add_gadget(fp, "pht_tag_bits_xor_%d_%d_%d", inject_target, branch_align,
                   dummy_branches);
      }
    }
  }
  return 0;
}
