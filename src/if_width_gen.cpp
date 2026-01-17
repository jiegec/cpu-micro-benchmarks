// if_width: measure instruction fetch width
// This benchmark measures how many instructions the frontend can fetch by
// crossing page boundaries.
// Pattern 0 places the last instruction of a page at the boundary.
// Pattern 1 is aligned control to compare performance.
// Performance degradation indicates the fetch unit cannot efficiently handle
// page boundary crossing.
//
// if_width: 测量指令获取宽度
// 此基准测试测量前端在跨越页边界时可以获取多少条指令。
// 模式 0 在页边界放置最后一条指令。
// 模式 1 是对齐的对照组，用于比较性能。
// 性能下降表明取指单元无法有效处理页边界跨越。

// ref:
// https://zhuanlan.zhihu.com/p/720136752

#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  // size: number of instructions in the second page for pattern 0
  int min_size = 2;
  int max_size = 64;
  int min_pattern = 0;
  int max_pattern = 1;

  fprintf(fp, ".text\n");
  for (int pattern = min_pattern; pattern <= max_pattern; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      fprintf(fp, ".global if_width_%d_%d\n", pattern, size);
      fprintf(fp, "if_width_%d_%d:\n", pattern, size);
#if defined(HOST_AMD64)
      int page_size = 16384;
      fprintf(fp, ".balign %d\n", page_size);
      if (pattern == 0) {
        fprintf(fp, "\t.rept %d\n", page_size - 1);
        fprintf(fp, "\tnop\n");
        fprintf(fp, "\t.endr\n");

        // put one nop as the last instruction in page
        fprintf(fp, "1:\n");
        fprintf(fp, "\tnop\n");

        // if the frontend can fetch the following instructions in one cycle,
        // performance is good
        for (int i = 0; i < size - 2; i++) {
          fprintf(fp, "\tnop\n");
        }
      } else {
        // make fetch aligned, execute the same count of instructions as
        // previous pattern
        fprintf(fp, "1:\n");
        for (int i = 0; i < size - 1; i++) {
          fprintf(fp, "\tnop\n");
        }
      }

      fprintf(fp, "\tdec %%rdi\n");
      fprintf(fp, "\tjne 1b\n");

      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      int page_size = 16384;
      fprintf(fp, ".balign %d\n", page_size);
      if (pattern == 0) {
        fprintf(fp, "\t.rept %d\n", (page_size - 4) / 4);
        fprintf(fp, "\tnop\n");
        fprintf(fp, "\t.endr\n");

        // put one nop as the last instruction in page
        fprintf(fp, "1:\n");
        fprintf(fp, "\tnop\n");

        // if the frontend can fetch the following instructions in one cycle,
        // performance is good
        for (int i = 0; i < size - 3; i++) {
          fprintf(fp, "\tnop\n");
        }
      } else {
        // make fetch aligned, execute the same count of instructions as
        // previous pattern
        fprintf(fp, "1:\n");
        for (int i = 0; i < size - 2; i++) {
          fprintf(fp, "\tnop\n");
        }
      }

      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tbne 1b\n");

      fprintf(fp, "\tret\n");
#elif defined(HOST_PPC64LE)
      int page_size = 65536;
      fprintf(fp, ".align 4\n");
      fprintf(fp, ".balign %d\n", page_size);
      if (pattern == 0) {
        fprintf(fp, "\t.rept %d\n", (page_size - 4) / 4);
        fprintf(fp, "\tnop\n");
        fprintf(fp, "\t.endr\n");

        // put one nop as the last instruction in page
        fprintf(fp, "1:\n");
        fprintf(fp, "\tnop\n");

        // if the frontend can fetch the following instructions in one cycle,
        // performance is good
        for (int i = 0; i < size - 2; i++) {
          fprintf(fp, "\tnop\n");
        }
      } else {
        // make fetch aligned, execute the same count of instructions as
        // previous pattern
        fprintf(fp, "1:\n");
        for (int i = 0; i < size - 1; i++) {
          fprintf(fp, "\tnop\n");
        }
      }

      fprintf(fp, "\taddic. 3, 3, -1\n");
      fprintf(fp, "\tbne 1b\n");

      fprintf(fp, "\tblr\n");
#endif
    }
  }

  define_gadgets_array(fp, "if_width_gadgets");
  for (int pattern = min_pattern; pattern <= max_pattern; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      add_gadget(fp, "if_width_%d_%d", pattern, size);
    }
  }
  return 0;
}
