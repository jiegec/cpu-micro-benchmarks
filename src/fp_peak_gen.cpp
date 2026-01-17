// fp_peak: measure floating-point peak performance
// This benchmark measures the peak floating-point performance by executing
// FMA (fused multiply-add) instructions in various precisions and widths:
// - Pattern 0: single precision scalar
// - Pattern 1: double precision scalar
// - Pattern 2: single precision 128-bit SIMD
// - Pattern 3: double precision 128-bit SIMD
// - Pattern 4: single precision 256-bit SIMD (AVX)
// - Pattern 5: double precision 256-bit SIMD (AVX)
// Results show FLOPs/cycle capability.
//
// fp_peak: 测量浮点峰值性能
// 此基准测试通过以各种精度和宽度执行 FMA（融合乘加）指令来测量峰值浮点性能：
// - 模式 0：单精度标量
// - 模式 1：双精度标量
// - 模式 2：单精度 128 位 SIMD
// - 模式 3：双精度 128 位 SIMD
// - 模式 4：单精度 256 位 SIMD (AVX)
// - 模式 5：双精度 256 位 SIMD (AVX)
// 结果是每周期 FLOPs。

#include "include/utils.h"
#include <cstdio>

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
#ifdef HOST_AARCH64
  int num_patterns = 6;
#else
  int num_patterns = 4;
#endif
  int repeat = 1000;

  // args: loop count
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    // entry
    fprintf(fp, ".global fp_peak_%d\n", pattern);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "fp_peak_%d:\n", pattern);
#if defined(HOST_AARCH64)
    fprintf(fp, "\tsub sp, sp, #0x100\n");
    fprintf(fp, "\tstp q8, q9, [sp, #0x0]\n");
    fprintf(fp, "\tstp q10, q11, [sp, #0x20]\n");
    fprintf(fp, "\tstp q12, q13, [sp, #0x40]\n");
    fprintf(fp, "\tstp q14, q15, [sp, #0x60]\n");

    if (pattern == 4 || pattern == 5) {
      fprintf(fp, "\t.arch armv9-a+sve\n");
      fprintf(fp, "\tptrue p0.b\n");
    }

    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      if (pattern == 0) {
        // single precision 32-bit fma using FMADD
        fprintf(fp, "\tfmadd s%d, s0, s1, s2\n", (i % 16) + 3);
      } else if (pattern == 1) {
        // double precision 64-bit fma using FMADD
        fprintf(fp, "\tfmadd d%d, d0, d1, d2\n", (i % 16) + 3);
      } else if (pattern == 2) {
        // single precision 128-bit fma using ASIMD
        fprintf(fp, "\tfmla v%d.4s, v0.4s, v1.4s\n", (i % 16) + 2);
      } else if (pattern == 3) {
        // double precision 128-bit fma using ASIMD
        fprintf(fp, "\tfmla v%d.2d, v0.2d, v1.2d\n", (i % 16) + 2);
      } else if (pattern == 4) {
        // single precision fma using SVE
        fprintf(fp, "\tfmla z%d.s, p0/m, z0.s, z1.s\n", (i % 16) + 2);
      } else if (pattern == 5) {
        // double precision fma using SVE
        fprintf(fp, "\tfmla z%d.d, p0/m, z0.d, z1.d\n", (i % 16) + 2);
      }
    }

    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    fprintf(fp, "\tldp q8, q9, [sp, #0x0]\n");
    fprintf(fp, "\tldp q10, q11, [sp, #0x20]\n");
    fprintf(fp, "\tldp q12, q13, [sp, #0x40]\n");
    fprintf(fp, "\tldp q14, q15, [sp, #0x60]\n");
    fprintf(fp, "\tadd sp, sp, #0x100\n");
    fprintf(fp, "\tret\n");
#endif
#if defined(HOST_AMD64)
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      if (pattern == 0) {
        // single precision 256-bit fma using FMA
        fprintf(fp, "\tvfmadd213ps %%ymm0, %%ymm1, %%ymm%d\n", (i % 14) + 2);
      } else if (pattern == 1) {
        // double precision 256-bit fma using FMA
        fprintf(fp, "\tvfmadd213pd %%ymm0, %%ymm1, %%ymm%d\n", (i % 14) + 2);
      } else if (pattern == 2) {
        // single precision 512-bit fma using AVX512F
        fprintf(fp, "\tvfmadd213ps %%zmm0, %%zmm1, %%zmm%d\n", (i % 14) + 2);
      } else if (pattern == 3) {
        // double precision 512-bit fma using AVX512F
        fprintf(fp, "\tvfmadd213pd %%zmm0, %%zmm1, %%zmm%d\n", (i % 14) + 2);
      }
    }

    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tjnz 1b\n");

    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "fp_peak_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    add_gadget(fp, "fp_peak_%d", pattern);
  }
  return 0;
}
