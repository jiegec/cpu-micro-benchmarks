// fetch_bandwidth: measure instruction fetch bandwidth
// This benchmark measures how many bytes per cycle the processor can fetch from
// I-cache. It creates loops with varying code sizes and measures the IPC.
// Results indicate the fetch bandwidth (e.g., 16 bytes/cycle means 4
// instructions/cycle for 4-byte instructions).
//
// fetch_bandwidth: 测量取指带宽
// 此基准测试测量处理器每周期可以从指令缓存获取多少字节。
// 它创建具有不同代码大小的循环并测量 IPC。
// 结果对应取指带宽（例如，16 字节/周期意味着对于 4 字节指令为 4 条指令/周期）。

#include "include/utils.h"
#include <set>
#include <vector>

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 1024;
  int max_size = 1048576;
  std::vector<int> mults = {1, 3, 5, 7, 9};
  std::set<int> sizes;
  for (int size = min_size; size <= max_size; size *= 2) {
    for (int mult : mults) {
      if (size * mult <= max_size) {
        sizes.insert(size * mult);
      }
    }
  }

  // args: loop count
  fprintf(fp, ".text\n");
  for (int size : sizes) {
#if defined(HOST_AMD64)
    fprintf(fp, ".global fetch_bandwidth_%d\n", size);
    fprintf(fp, "fetch_bandwidth_%d:\n", size);

    fprintf(fp, "\t.balign 4096\n");
    fprintf(fp, "fetch_bandwidth_%d_loop_begin:\n", size);

    for (int i = 0; i < size / 4 - 2; i++) {
      // 4 bytes nop
      emit_multibyte_nops(fp, 4);
      // 8 bytes nop
      // emit_multibyte_nops(fp, 8);
    }
    fprintf(fp, "\tdec %%rdi\n");
    // 6 bytes jnz
    fprintf(fp, "\t.byte 0x0f\n");
    fprintf(fp, "\t.byte 0x85\n");
    fprintf(fp, "\t.long fetch_bandwidth_%d_loop_begin - . - 4\n", size);

    fprintf(fp, "\tret\n");

#elif defined(HOST_AARCH64)
    fprintf(fp, ".global fetch_bandwidth_%d\n", size);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "fetch_bandwidth_%d:\n", size);

    fprintf(fp, "\t.balign 4096\n");
    fprintf(fp, "\t1:\n");

    for (int i = 0; i < size / 4 - 2; i++) {
      fprintf(fp, "\tnop\n");
    }
    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    fprintf(fp, "\tret\n");
#elif defined(HOST_LOONGARCH64)
    fprintf(fp, ".global fetch_bandwidth_%d\n", size);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "fetch_bandwidth_%d:\n", size);

    fprintf(fp, "\t.balign 4096\n");
    fprintf(fp, "\t1:\n");

    for (int i = 0; i < size / 4 - 2; i++) {
      fprintf(fp, "\tnop\n");
    }
    fprintf(fp, "\taddi.d $a0, $a0, -1\n");
    fprintf(fp, "\tbne $a0, $zero, 1b\n");

    fprintf(fp, "\tret\n");
#elif defined(HOST_PPC64LE)
    fprintf(fp, ".global fetch_bandwidth_%d\n", size);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "fetch_bandwidth_%d:\n", size);

    fprintf(fp, "\t.balign 4096\n");
    fprintf(fp, "\t1:\n");

    for (int i = 0; i < size / 4 - 4; i++) {
      fprintf(fp, "\tnop\n");
    }
    fprintf(fp, "\taddi 3, 3, -1\n");
    fprintf(fp, "\tcmpdi CR0, 3, 0\n");
    fprintf(fp, "\tbeq 2f\n");
    fprintf(fp, "\tb 1b\n");

    fprintf(fp, "\t2:\n");
    fprintf(fp, "\tblr\n");
#endif
  }

  define_gadgets_array(fp, "fetch_bandwidth_gadgets");
  for (int size : sizes) {
    add_gadget(fp, "fetch_bandwidth_%d", size);
  }
  return 0;
}
