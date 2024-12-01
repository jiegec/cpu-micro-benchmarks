#include "include/utils.h"
#include <cstdint>
#include <set>
#include <vector>

// btb_size_basic: measure btb size and latency
// BTB saves information for branch instructions. To measure btb size and
// latency, we fill btb with lots of branch instructions. By measuring cycles
// per branch, we can find the btb size and the latency. Since btb are usually
// set associative, we span the branch instructions in the memory by constant
// stride. If the btb capacity is reduced as the stride increases, we can know
// that lower bits of btb index are set to zero due to the branch placement. The
// performance can be plotted by running `python3
// figures/plot_btb_size_basic.py` and the figure is located at
// `plot_btb_size_basic.png`. You may need to customize the strides displayed in
// the figure to reduce noise. btb_size_basic: 测量 btb 大小和延迟 BTB
// 保存了分支指令的信息。为了测量 BTB 的的大小和延迟，我们用大量的分支指令来填满
// BTB。因为 BTB
// 通常是组相连的，我们在内存中把分支指令按固定间距隔开。如果随着间距增长，BTB
// 的容量随之减少，我们就知道 BTB index 的低位被设置为零了。为了查看 BTB
// 的容量和性能，运行 `python3 figures/plot_btb_size_basic.py`，并查看
// `plot_btb_size_basic.png` 图片。可能需要修改图片显示的间距以减少噪声。

// ref:
// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/tests/BtbTest.cs
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  uint64_t min_size = 2;
  uint64_t max_size = 65536;
  // limited binary size on ios
  uint64_t max_product = 32768;
  uint64_t min_stride = 4;
  // limited binary size on ios
  uint64_t max_stride = 8192;
  std::vector<uint64_t> mults = {1, 3, 5, 7};
  int num_patterns = 3;
  // pattern 0: unconditional branch
  // pattern 1: conditional branch
  // pattern 2: mixed unconditional + conditional branch

  // args: loop count
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (uint64_t stride = min_stride; stride <= max_stride; stride *= 2) {
      std::set<uint64_t> sizes;
      for (uint64_t size_base = min_size; size_base <= max_product / stride;
           size_base *= 2) {
        for (uint64_t mult : mults) {
          for (uint64_t size = size_base * mult - 1;
               size <= size_base * mult + 1 && size * stride <= max_product &&
               size <= max_size;
               size++) {
            sizes.insert(size);
          }
        }
      }

      for (uint64_t size : sizes) {
        fprintf(fp, ".global btb_size_basic_%d_%ld_%ld\n", pattern, size,
                stride);
        fprintf(fp, ".balign %ld\n", stride);
        fprintf(fp, "btb_size_basic_%d_%ld_%ld:\n", pattern, size, stride);
#ifdef HOST_AARCH64
        fprintf(fp, "\t1:\n");
        // the last loop is bne 1b
        for (int i = 0; i < (int)size - 1; i++) {
          if (stride > 4) {
            // place b 2f at the same index as last bne
            fprintf(fp, "\tnop\n");
          }
          if (pattern == 0 || (pattern == 2 && i % 2 == 0)) {
            // unconditional
            fprintf(fp, "\tb 2f\n");
          } else if (pattern == 1 || (pattern == 2 && i % 2 == 1)) {
            // conditional
            fprintf(fp, "\tcbnz x0, 2f\n");
          }
          // fill nops so that branch instructions have the specified stride
          fprintf(fp, "\t.balign %ld\n", stride);
          fprintf(fp, "\t2:\n");
        }
        fprintf(fp, "\tsubs x0, x0, #1\n");
        fprintf(fp, "\tbne 1b\n");
        fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
        fprintf(fp, "\t1:\n");
        // the last loop is bne 1b
        for (int i = 0; i < (int)size - 1; i++) {
          if (pattern == 0 || (pattern == 2 && i % 2 == 0)) {
            // unconditional
            fprintf(fp, "\tjmp 2f\n");
          } else if (pattern == 1 || (pattern == 2 && i % 2 == 1)) {
            // conditional
            fprintf(fp, "\tjnz 2f\n");
          }
          // fill nops so that branch instructions have the specified stride
          fprintf(fp, "\t.balign %ld\n", stride);
          fprintf(fp, "\t2:\n");
        }
        fprintf(fp, "\tdec %%rdi\n");
        fprintf(fp, "\tjne 1b\n");
        fprintf(fp, "\tret\n");
#endif
      }
    }
  }

  define_gadgets_array(fp, "btb_size_basic_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (uint64_t stride = min_stride; stride <= max_stride; stride *= 2) {
      std::set<uint64_t> sizes;
      for (uint64_t size_base = min_size; size_base <= max_product / stride;
           size_base *= 2) {
        for (uint64_t mult : mults) {
          for (uint64_t size = size_base * mult - 1;
               size <= size_base * mult + 1 && size * stride <= max_product &&
               size <= max_size;
               size++) {
            sizes.insert(size);
          }
        }
      }

      for (uint64_t size : sizes) {
        add_gadget(fp, "btb_size_basic_%d_%ld_%ld", pattern, size, stride);
      }
    }
  }
  return 0;
}
