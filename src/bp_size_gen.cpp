// bp_size: test branch prediction capability
// Generate several branches, and each branch follow its own patterns. Observe
// performance, if the branch predictor fails to predict so many branches to so
// long history, the performance will be slower. The result is qualitative. You
// can see the result via `python3 figures/plot_bp_size.py` and viewing
// `plot_bp_size.png`.
// bp_size: 测试分支预测器的能力
// 生成若干个分支，每个分支按照自己的模式选择跳转或不跳转。
// 观察性能，如果分支预测器无法预测那么多的分支，或者每个分支无法预测那么长的历史，
// 性能会有明显的下降。这是一个定性的测试。
// 你可以通过运行 `python3 figures/plot_bp_size.py`
// 然后查看 `plot_bp_size.png` 图片来观察结果。

#include "include/utils.h"
#include <cstdio>

// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/tests/BranchHistoryTest.cs
// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/DataFiles/GccBranchHistFunction.c
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 1;
  int max_size = 2048;

  // args: loop count, pattern array[branch][history], history len
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size *= 2) {
    // entry
    fprintf(fp, ".global bp_size_%d\n", size);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "bp_size_%d:\n", size);
#ifdef HOST_AARCH64
    fprintf(fp, "\teor x16, x16, x16\n");
    fprintf(fp, "\teor x15, x15, x15\n");
    fprintf(fp, "\teor x12, x12, x12\n");
    fprintf(fp, "\teor x11, x11, x11\n");
    fprintf(fp, "\tmov x14, x1\n");

#ifndef __APPLE__
    // alignment cannot surpass page size
    fprintf(fp, ".balign %d\n", (1 << 19));
#endif
    fprintf(fp, "\t1:\n");

    // data-dependent branches
    // x14: pointer of pointer to history array
    // w16: history index
    for (int i = 0; i < size; i++) {
      // post-index
      fprintf(fp, "\tldr x15, [x14], #8\n");
      fprintf(fp, "\tldr w13, [x15, w16, uxtw #2]\n");
      // conditional branch
      fprintf(fp, "\tcbnz x13, 2f\n");
      fprintf(fp, "\t2:\n");
    }

    // increment w16, set w16 to 0 if w16 == history len
    fprintf(fp, "\tadd w16, w16, 1\n");
    fprintf(fp, "\tcmp w16, w2\n");
    fprintf(fp, "\tcsel w16, w11, w16, EQ\n");
    // reset branch address
    fprintf(fp, "\tmov x14, x1\n");

    // x12 = 1b
    arm64_la(fp, 12, "1b");
    // x13 = 2f
    arm64_la(fp, 13, "2f");
    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tcsel x12, x12, x13, ne\n");
    fprintf(fp, "\tbr x12\n");
    fprintf(fp, "\t2:\n");

    fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
    // rdi: loop count
    // rsi: pattern array
    // rdx: history length

    fprintf(fp, "\tpush %%rbx\n");
    fprintf(fp, "\tmov $0, %%rbx\n");
    fprintf(fp, "\tmov $0, %%r11\n");
    fprintf(fp, "\t1:\n");
    fprintf(fp, "\tmov $0, %%r8\n");

    // data-dependent branches
    // r8: branch index
    // rbx: history index
    for (int i = 0; i < size; i++) {
      fprintf(fp, "\tmov 0(%%rsi, %%r8, 8), %%r9\n");
      fprintf(fp, "\tinc %%r8\n");
      fprintf(fp, "\tmov 0(%%r9, %%rbx, 4), %%rcx\n");
      // conditional branch
      fprintf(fp, "\tjecxz 2f\n");
      fprintf(fp, "\t2:\n");
    }

    // increment rbx, set rbx to 0 if rbx == history len
    fprintf(fp, "\tinc %%rbx\n");
    fprintf(fp, "\tcmp %%rbx, %%rdx\n");
    fprintf(fp, "\tcmove %%r11, %%rbx\n");

    // r8 = 1b
    fprintf(fp, "\tlea 1b(%%rip), %%r8\n");
    // r9 = 2f
    fprintf(fp, "\tlea 2f(%%rip), %%r9\n");
    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tcmovz %%r9, %%r8\n");
    fprintf(fp, "\tjmp *%%r8\n");
    fprintf(fp, "\t2:\n");

    fprintf(fp, "\tpop %%rbx\n");
    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "bp_size_gadgets");
  for (int size = min_size; size <= max_size; size *= 2) {
    add_gadget(fp, "bp_size_%d", size);
  }
  return 0;
}
