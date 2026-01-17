// ghr_size: measure global history register size
// Global history register (GHR) stores the history of branch directions for
// branch prediction. To measure GHR size, we create a sequence of always-taken
// branches followed by one data-dependent branch. When the number of
// always-taken branches exceeds GHR size, older branch history is lost, and the
// final branch cannot be predicted correctly using the full history.
//
// ghr_size: 测量全局历史寄存器的大小
// 全局历史寄存器（GHR）存储分支方向的历史记录用于分支预测。
// 为了测量 GHR 的大小，我们创建一系列总是执行的分支，后跟一个数据依赖的分支。
// 当总是执行的分支数量超过 GHR 大小时，较旧的分支历史会丢失，
// 最终分支无法使用完整历史进行正确预测。

// ref:
// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf

#include "include/utils.h"
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 1;
  int max_size = 1024;

  // args: loop count, random array
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    // always taken or not taken for dummy branches
    fprintf(fp, ".global ghr_size_%d\n", size);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "ghr_size_%d:\n", size);
#ifdef HOST_AARCH64
    fprintf(fp, "\t1:\n");

    // always taken branches ahead
    for (int i = 0; i < size - 2; i++) {
      fprintf(fp, "\tcbnz x1, 2f\n");
      // alignment is required, otherwise too many branches in a cache line
      fprintf(fp, "\t.balign 32\n");
      fprintf(fp, "\t2:\n");
    }

    // taken/not taken based on x0 & 1
    fprintf(fp, "\tand x2, x0, #1\n");
    fprintf(fp, "\tcbnz x2, 2f\n");
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

    // always taken branches ahead
    fprintf(fp, "\tmov $1, %%rcx\n");
    fprintf(fp, "\ttest %%rcx, %%rcx\n");
    for (int i = 0; i < size - 2; i++) {
      fprintf(fp, "\tjnz 2f\n");
      // alignment is required, otherwise too many branches in a cache line
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\t2:\n");
      fprintf(fp, "\tnop\n");
      fprintf(fp, "\t.balign 64\n");
    }

    // taken/not taken based on rdi & 1
    fprintf(fp, "\tmov %%rdi, %%rcx\n");
    fprintf(fp, "\tand $1, %%rcx\n");
    fprintf(fp, "\ttest %%rcx, %%rcx\n");
    fprintf(fp, "\tjnz 2f\n");
    fprintf(fp, "\t.balign 64\n");
    fprintf(fp, "\t2:\n");
    fprintf(fp, "\tnop\n");
    fprintf(fp, "\t.balign 64\n");

    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tjnz 1b\n");

    // restore regs
    fprintf(fp, "\tpop %%rcx\n");
    fprintf(fp, "\tpop %%rbx\n");
    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "ghr_size_gadgets");
  for (int size = min_size; size <= max_size; size++) {
    add_gadget(fp, "ghr_size_%d", size);
  }
  return 0;
}
