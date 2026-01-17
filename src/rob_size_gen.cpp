// rob_size: measure reorder buffer size
// Reorder buffer (ROB) holds instructions that are executing but not yet
// retired. To measure ROB size, we create a chain of dependent loads (long
// latency) separated by varying numbers of independent instructions. When the
// number of independent instructions exceeds ROB size, the dependent load
// cannot start until the previous load completes, causing performance
// degradation.
//
// rob_size: 测试 ROB 的大小
// 重排序缓冲区（ROB）保存正在执行但尚未完成的指令。
// 为了测量 ROB 的大小，我们创建一条由有依赖的加载指令（长延迟）组成的链，
// 链中用不同数量的独立指令分隔。
// 当独立指令的数量超过 ROB 大小时，依赖的加载指令无法在前一条加载完成前开始，
// 导致性能下降。

#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int repeat = 20;
  int min_size = 1;
  int max_size = 1024;
  // args: loop count, buffer
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    fprintf(fp, ".global rob_size_%d\n", size);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "rob_size_%d:\n", size);
#ifdef HOST_AARCH64
    int sqrt_count = 8;
    bool use_sqrt = false;
    fprintf(fp, "\tldr x3, [x0]\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      if (use_sqrt) {
        // use sqrt if necessary
        for (int j = 0; j < sqrt_count; j++) {
          fprintf(fp, "\tfsqrt d0, d0\n");
        }
      } else {
        fprintf(fp, "\tldr x3, [x3]\n");
      }
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
        // use load to measure rob group count on apple cores
        // fprintf(fp, "\tldr x4, [x3]\n");
      }
    }
    fprintf(fp, "\tsubs x2, x2, #1\n");
    fprintf(fp, "\tbne 1b\n");
    fprintf(fp, "\tstr x3, [x0]\n");
    fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
    fprintf(fp, "\tmovq 0(%%rdi), %%r8\n");
    fprintf(fp, "\tmovq 0(%%rsi), %%r9\n");
    fprintf(fp, "\tmovq %%rdx, %%rax\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tmovq (%%r8), %%r8\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
      fprintf(fp, "\tmovq (%%r9), %%r9\n");
      // forbit further speculation
      fprintf(fp, "\tlfence\n");
      fprintf(fp, "\tmfence\n");
    }
    fprintf(fp, "\tsubl $1, %%eax\n");
    fprintf(fp, "\tjne 1b\n");
    fprintf(fp, "\tmovq %%r8, 0(%%rdi)\n");
    fprintf(fp, "\tmovq %%r9, 0(%%rsi)\n");
    fprintf(fp, "\tret\n");
#elif defined(__loongarch__)
    fprintf(fp, "\tld.d $a3, $a0, 0\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tld.d $a3, $a3, 0\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
    }
    fprintf(fp, "\taddi.d $a2, $a2, -1\n");
    fprintf(fp, "\tbne $a2, $zero, 1b\n");
    fprintf(fp, "\tst.d $a3, $a0, 0\n");
    fprintf(fp, "\tret\n");
#elif defined(HOST_PPC64LE)
    fprintf(fp, "\tld 6, 0(3)\n");
    fprintf(fp, "\tld 7, 0(4)\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tld 6, 0(6)\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
      fprintf(fp, "\tld 7, 0(7)\n");
      // forbid further speculation
      fprintf(fp, "\tsync\n");
      fprintf(fp, "\thwsync\n");
    }
    // loop
    fprintf(fp, "\taddic. 5, 5, -1\n");
    // use b for longer distance
    fprintf(fp, "\tbeq 2f\n");
    fprintf(fp, "\tb 1b\n");
    fprintf(fp, "\t2:\n");
    fprintf(fp, "\tstd 6, 0(3)\n");
    fprintf(fp, "\tstd 7, 0(4)\n");
    fprintf(fp, "\tblr\n");
#endif
  }

  define_gadgets_array(fp, "rob_size_gadgets");
  for (int size = min_size; size <= max_size; size++) {
    add_gadget(fp, "rob_size_%d", size);
  }
  return 0;
}
