#include "include/utils.h"

// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf
// https://arxiv.org/pdf/2411.13900
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_branches = 1;
  int max_branches = 32;
  int min_branch_align = 3;
#ifdef __APPLE__
  // alignment cannot surpass page size
  int max_branch_align = 8;
#else
  int max_branch_align = 19;
#endif
// this bit should be part of PHT index
#ifdef QUALCOMM_ORYON
  int third_bit = 95;
#else
  int third_bit = 99;
#endif

  // args: loop count, random array
  fprintf(fp, ".text\n");
  for (int branches = min_branches; branches <= max_branches; branches++) {
    for (int branch_align = min_branch_align; branch_align <= max_branch_align;
         branch_align++) {
      fprintf(fp, ".global pht_associativity_%d_%d\n", branches, branch_align);
      fprintf(fp, "pht_associativity_%d_%d:\n", branches, branch_align);
#ifdef HOST_AARCH64
      fprintf(fp, "\tpht_associativity_%d_%d_loop_begin:\n", branches,
              branch_align);

      // read random value
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");

      // loop to shift phr
      for (int i = 0; i < 200; i++) {
        fprintf(fp, "\tb pht_associativity_%d_%d_dummy_target1_%d\n", branches,
                branch_align, i);
        fprintf(fp, "\tpht_associativity_%d_%d_dummy_target1_%d:\n", branches,
                branch_align, i);
      }

      // inject w11 to phr LSB
      arm64_la(fp, 12, "pht_associativity_%d_%d_first_target_1", branches,
               branch_align);
      arm64_la(fp, 13, "pht_associativity_%d_%d_first_target_2", branches,
               branch_align);
      fprintf(fp, "\tcmp w11, 0\n");
      fprintf(fp, "\tcsel x12, x12, x13, eq\n");
      fprintf(fp, "\tbr x12\n");
      // T[2] differs
      fprintf(fp, "\t.balign 8\n");
      fprintf(fp, "\tpht_associativity_%d_%d_first_target_1:\n", branches,
              branch_align);
      fprintf(fp, "\tnop\n");
      fprintf(fp, "\tpht_associativity_%d_%d_first_target_2:\n", branches,
              branch_align);

      // loop to shift phr by third_bit-3/third_bit-4 times
      // we have 2/3 branches afterwards
      bool extra_jump = branch_align >= 8;
      for (int i = 0; i < third_bit - (extra_jump ? 3 : 2); i++) {
        fprintf(fp, "\tb pht_associativity_%d_%d_dummy_target2_%d\n", branches,
                branch_align, i);
        fprintf(fp, "\tpht_associativity_%d_%d_dummy_target2_%d:\n", branches,
                branch_align, i);
      }

      // jump to stage1 branches
      // 4 to target1, 4 to target2, etc
      // (x0 / 4) % branches -> stage1 branch index
      // x13 = x0 / 4
      fprintf(fp, "\tlsr x13, x0, 0x2\n");
      // x14 = (x0 / 4) / branches
      fprintf(fp, "\tmov x15, %d\n", branches);
      fprintf(fp, "\tudiv x14, x13, x15\n");
      // x12 = (x0 / 4) % branches
      fprintf(fp, "\tmsub x12, x14, x15, x13\n");
      // x16 = stage1_branches
      arm64_la(fp, 16, "pht_associativity_%d_%d_stage1_branches", branches,
               branch_align);
      // load branch address
      fprintf(fp, "\tldr x12, [x16, x12, lsl #3]\n");
      fprintf(fp, "\tbr x12\n");

      // stage1 branches
      fprintf(fp, "\t.balign %d\n", (1 << std::min(branch_align + 5, 20)));
      for (int i = 0; i < branches; i++) {
        if (extra_jump) {
          // target differs in T[6] * i
          fprintf(fp, "\t.balign %d\n", (1 << (branch_align - 2)));
        } else {
          // target differs in T[7] * i
          fprintf(fp, "\t.balign %d\n", (1 << (branch_align - 1)));
        }
        fprintf(fp, "\tpht_associativity_%d_%d_stage1_%d:\n", branches,
                branch_align, i);
        // do not change B[5:2]
        if (extra_jump) {
          fprintf(fp, "\tb pht_associativity_%d_%d_stage1_end\n", branches,
                  branch_align);
        } else {
          fprintf(fp, "\tnop\n");
        }
      }
      fprintf(fp, "\tpht_associativity_%d_%d_stage1_end:\n", branches,
              branch_align);

      // jump to stage2 branches
      // 4 to target1, 4 to target2, etc
      // (x0 / 4) % branches -> stage2 branch index
      // x13 = x0 / 4
      fprintf(fp, "\tlsr x13, x0, 0x2\n");
      // x14 = (x0 / 4) / branches
      fprintf(fp, "\tmov x15, %d\n", branches);
      fprintf(fp, "\tudiv x14, x13, x15\n");
      // x12 = (x0 / 4) % branches
      fprintf(fp, "\tmsub x12, x14, x15, x13\n");
      // x16 = stage2_branches
      arm64_la(fp, 16, "pht_associativity_%d_%d_stage2_branches", branches,
               branch_align);
      // load branch address
      fprintf(fp, "\tldr x12, [x16, x12, lsl #3]\n");
      fprintf(fp, "\tbr x12\n");

      // stage2 branches
      // target differs in T[8] * i
      fprintf(fp, "\t.balign %d\n", (1 << std::min(branch_align + 6, 20)));
      for (int i = 0; i < branches; i++) {
        fprintf(fp, "\t.balign %d\n", (1 << branch_align));
        fprintf(fp, "\tpht_associativity_%d_%d_stage2_%d:\n", branches,
                branch_align, i);
        // detect index & tag collision
        if (i == 0)
          fprintf(fp, "\tcbnz x11, pht_associativity_%d_%d_stage3_%d\n",
                  branches, branch_align, i);
        else
          fprintf(fp, "\tcbz x11, pht_associativity_%d_%d_stage3_%d\n",
                  branches, branch_align, i);
        fprintf(fp, "\tpht_associativity_%d_%d_stage3_%d:\n", branches,
                branch_align, i);
        fprintf(fp, "\tb pht_associativity_%d_%d_branch_end\n", branches,
                branch_align);
      }

      // save stage1 branches
      fprintf(fp, "\t.data\n");
      fprintf(fp, "\t.balign 8\n");
      fprintf(fp, "\tpht_associativity_%d_%d_stage1_branches:\n", branches,
              branch_align);
      for (int i = 0; i < branches; i++) {
        fprintf(fp, "\t.dword pht_associativity_%d_%d_stage1_%d\n", branches,
                branch_align, i);
      }

      // save stage2 branches
      fprintf(fp, "\t.balign 8\n");
      fprintf(fp, "\tpht_associativity_%d_%d_stage2_branches:\n", branches,
              branch_align);
      for (int i = 0; i < branches; i++) {
        fprintf(fp, "\t.dword pht_associativity_%d_%d_stage2_%d\n", branches,
                branch_align, i);
      }
      fprintf(fp, "\t.text\n");

      fprintf(fp, "\tpht_associativity_%d_%d_branch_end:\n", branches,
              branch_align);

      // loop end
      arm64_la(fp, 12, "pht_associativity_%d_%d_end_loop", branches,
               branch_align);
      arm64_la(fp, 13, "pht_associativity_%d_%d_loop_begin", branches,
               branch_align);
      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tcsel x12, x12, x13, eq\n");
      fprintf(fp, "\tbr x12\n");
      fprintf(fp, "\tpht_associativity_%d_%d_end_loop:\n", branches,
              branch_align);

      fprintf(fp, "\tret\n");
#endif
    }
  }

  define_gadgets_array(fp, "pht_associativity_gadgets");
  for (int branches = min_branches; branches <= max_branches; branches++) {
    for (int branch_align = min_branch_align; branch_align <= max_branch_align;
         branch_align++) {
      add_gadget(fp, "pht_associativity_%d_%d", branches, branch_align);
    }
  }
  return 0;
}
