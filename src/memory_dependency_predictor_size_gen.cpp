// memory_dependency_predictor_size: measure memory dependency predictor size
// This benchmark measures the size of the memory dependency predictor that
// tracks store-to-load forwarding. It creates sequences of stores followed by
// loads with various dependency patterns (data dependency, address dependency,
// no dependency). When the sequence exceeds the predictor size, performance
// degrades.
//
// memory_dependency_predictor_size: 测量内存依赖预测器的大小
// 此基准测试测量跟踪Store到Load转发的内存依赖预测器的大小。
// 它创建Store序列，后跟具有各种依赖模式（数据依赖、地址依赖、无依赖）的加载。
// 当序列超过预测器大小时，性能会下降。

#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 1;
  int max_size = 128;
  int num_patterns = 5;
  // args: loop count, ptr1, ptr2
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      fprintf(fp, ".global memory_dependency_predictor_size_%d_%d\n", pattern,
              size);
      fprintf(fp, ".balign 64\n");
      fprintf(fp, "memory_dependency_predictor_size_%d_%d:\n", pattern, size);
#if defined(HOST_AMD64)
      fprintf(fp, "\t1:\n");

      for (int i = 0; i < size; i++) {
        // 64-bit store, 64-bit load
        if (pattern == 0) {
          // dependency on data
          fprintf(fp, "\tmov %%rcx, 0(%%rsi)\n");
          fprintf(fp, "\tmov 0(%%rdx), %%rcx\n");
        } else if (pattern == 1) {
          // dependency on data, 1 st + 2 ld
          fprintf(fp, "\tmov %%rcx, 0(%%rsi)\n");
          fprintf(fp, "\tmov 0(%%rdx), %%rcx\n");
          fprintf(fp, "\tmov 0(%%rdx), %%rcx\n");
        } else if (pattern == 2) {
          // dependency on addr
          fprintf(fp, "\tmov %%rdx, 0(%%rsi)\n");
          fprintf(fp, "\tmov 0(%%rdx), %%rsi\n");
        } else if (pattern == 3) {
          // dependency on addr, 1 st + 2 ld
          fprintf(fp, "\tmov %%rdx, 0(%%rsi)\n");
          fprintf(fp, "\tmov 0(%%rdx), %%rsi\n");
          fprintf(fp, "\tmov 0(%%rdx), %%rsi\n");
        } else if (pattern == 4) {
          // no dependency
          fprintf(fp, "\tmov %%rbx, 0(%%rsi)\n");
          fprintf(fp, "\tmov 0(%%rdx), %%rcx\n");
        }
      }

      fprintf(fp, "\tdec %%rdi\n");
      fprintf(fp, "\tjne 1b\n");

      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      fprintf(fp, "\t1:\n");

      for (int i = 0; i < size; i++) {
        // 64-bit store, 64-bit load
        if (pattern == 0) {
          // dependency on data
          fprintf(fp, "\tstr x3, [x1]\n");
          fprintf(fp, "\tldr x3, [x2]\n");
        } else if (pattern == 1) {
          // dependency on data, 1 st + 2 ld
          fprintf(fp, "\tstr x3, [x1]\n");
          fprintf(fp, "\tldr x3, [x2]\n");
          fprintf(fp, "\tldr x3, [x2]\n");
        } else if (pattern == 2) {
          // dependency on addr
          fprintf(fp, "\tstr x2, [x1]\n");
          fprintf(fp, "\tldr x1, [x2]\n");
        } else if (pattern == 3) {
          // dependency on addr, 1 st + 2 ld
          fprintf(fp, "\tstr x2, [x1]\n");
          fprintf(fp, "\tldr x1, [x2]\n");
          fprintf(fp, "\tldr x1, [x2]\n");
        } else if (pattern == 4) {
          // no dependency
          fprintf(fp, "\tstr x4, [x1]\n");
          fprintf(fp, "\tldr x3, [x2]\n");
        }
      }

      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tbne 1b\n");

      fprintf(fp, "\tret\n");
#endif
    }
  }

  define_gadgets_array(fp, "memory_dependency_predictor_size_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      add_gadget(fp, "memory_dependency_predictor_size_%d_%d", pattern, size);
    }
  }
  return 0;
}
