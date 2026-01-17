// ras_size: measure return address stack size
// Return address stack (RAS) is used to predict return targets for function
// returns. To measure RAS size, we create recursive function calls with various
// depths. When the depth exceeds RAS size, performance degrades because the
// stack overflows and the processor cannot predict the return target correctly.
// Two variants are tested: variant 0 allows BTB to predict, variant 1 forces
// RAS prediction.
//
// ras_size: 测量返回地址栈的大小
// 返回地址栈（RAS）用于预测函数返回的目标地址。
// 为了测量 RAS 的大小，我们创建不同深度的递归函数调用。
// 当深度超过 RAS 大小时，性能会下降，因为栈溢出，处理器无法正确预测返回目标。
// 测试了两种变体：变体 0 允许 BTB 预测，变体 1 强制使用 RAS 预测。

// ref:
// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/tests/ReturnStackTest.cs

#include "include/utils.h"
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 1;
  int max_size = 128;
  int num_variant = 2;

  // args: loop count
  fprintf(fp, ".text\n");
  for (int variant = 0; variant < num_variant; variant++) {
    for (int size = min_size; size <= max_size; size++) {
      // entry
      fprintf(fp, ".global ras_size_%d_%d\n", variant, size);
      fprintf(fp, ".balign 64\n");
      fprintf(fp, "ras_size_%d_%d:\n", variant, size);
#ifdef HOST_AARCH64
      // save lr
      fprintf(fp, "\tsub sp, sp, #0x20\n");
      fprintf(fp, "\tstp x29, x30, [sp, #0x10]\n");

      fprintf(fp, "\t1:\n");
      // call function
      fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tbne 1b\n");

      // restore lr
      fprintf(fp, "\tldp x29, x30, [sp, #0x10]\n");
      fprintf(fp, "\tadd sp, sp, #0x20\n");
      fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
      fprintf(fp, "\t1:\n");
      // call function
      fprintf(fp, "\tcall ras_func_%d_%d\n", variant, size - 1);
      fprintf(fp, "\tdec %%rdi\n");
      fprintf(fp, "\tjne 1b\n");
      fprintf(fp, "\tret\n");
#elif defined(HOST_PPC64LE)
      // save lr
      fprintf(fp, "\taddi 1, 1, -32\n");
      fprintf(fp, "\tstd 31, 24(1)\n");
      fprintf(fp, "\tmflr 0\n");
      fprintf(fp, "\tstd 0, 16(1)\n");

      fprintf(fp, "\t1:\n");
      // call function
      fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
      // addic. updates the condition register,
      // cmp is no longer required
      fprintf(fp, "\taddic. 3, 3, -1\n");
      fprintf(fp, "\tbne 1b\n");

      // restore lr
      fprintf(fp, "\tld 0, 16(1)\n");
      fprintf(fp, "\tmtlr 0\n");
      fprintf(fp, "\tld 31, 24(1)\n");
      fprintf(fp, "\taddi 1, 1, 32\n");
      fprintf(fp, "\tblr\n");
#endif

      // inner function
      fprintf(fp, ".global ras_func_%d_%d\n", variant, size);
      fprintf(fp, ".balign 64\n");
      fprintf(fp, "ras_func_%d_%d:\n", variant, size);

      // variant #0:
      // each ret only goes to one call site, allowing BTB to generate
      // prediction

      // variant #1:
      // if we don't want BTB to predict target address for ret
      // we can use two bl, and alternate between the two using x0

#ifdef HOST_AARCH64
      // save lr
      fprintf(fp, "\tsub sp, sp, #0x20\n");
      fprintf(fp, "\tstp x29, x30, [sp, #0x10]\n");

      if (variant == 1) {
        fprintf(fp, "\tand x1, x0, #1\n");
        fprintf(fp, "\tcbz x1, 2f\n");
        fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
        fprintf(fp, "\tb 3f\n");

        fprintf(fp, "\t2:\n");
        fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
        fprintf(fp, "\tb 3f\n");
        fprintf(fp, "\t3:\n");

      } else {
        // call lower function
        fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
      }

      // restore lr
      fprintf(fp, "\tldp x29, x30, [sp, #0x10]\n");
      fprintf(fp, "\tadd sp, sp, #0x20\n");
      fprintf(fp, "\tret\n");
#elif defined(HOST_AMD64)
      if (variant == 1) {
        // use different return addresses to force RAS to provide the prediction
        fprintf(fp, "\tmov %%rdi, %%rsi\n");
        fprintf(fp, "\tand $1, %%rsi\n");
        fprintf(fp, "\tje 2f\n");
        fprintf(fp, "\tcall ras_func_%d_%d\n", variant, size - 1);
        fprintf(fp, "\tjmp 3f\n");
        fprintf(fp, "\t2:\n");
        fprintf(fp, "\tcall ras_func_%d_%d\n", variant, size - 1);
        fprintf(fp, "\tjmp 3f\n");
        fprintf(fp, "\t3:\n");
        fprintf(fp, "\tret\n");
      } else {
        fprintf(fp, "\tcall ras_func_%d_%d\n", variant, size - 1);
        fprintf(fp, "\tret\n");
      }
#elif defined(HOST_PPC64LE)
      // save lr
      fprintf(fp, "\taddi 1, 1, -32\n");
      fprintf(fp, "\tstd 31, 24(1)\n");
      fprintf(fp, "\tmflr 0\n");
      fprintf(fp, "\tstd 0, 16(1)\n");

      if (variant == 1) {
        fprintf(fp, "\tandi. 4, 3, 1\n");
        fprintf(fp, "\tbeq 2f\n");
        fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
        fprintf(fp, "\tb 3f\n");

        fprintf(fp, "\t2:\n");
        fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
        fprintf(fp, "\tb 3f\n");
        fprintf(fp, "\t3:\n");

      } else {
        // call lower function
        fprintf(fp, "\tbl ras_func_%d_%d\n", variant, size - 1);
      }

      // restore lr
      fprintf(fp, "\tld 0, 16(1)\n");
      fprintf(fp, "\tmtlr 0\n");
      fprintf(fp, "\tld 31, 24(1)\n");
      fprintf(fp, "\taddi 1, 1, 32\n");
      fprintf(fp, "\tblr\n");
#endif
    }

    // recursion base
    fprintf(fp, ".global ras_func_%d_%d\n", variant, 0);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "ras_func_%d_%d:\n", variant, 0);
#if defined(HOST_PPC64LE)
    fprintf(fp, "\tblr\n");
#else
    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp, "ras_size_gadgets");
  for (int variant = 0; variant < num_variant; variant++) {
    for (int size = min_size; size <= max_size; size++) {
      add_gadget(fp, "ras_size_%d_%d", variant, size);
    }
  }
  return 0;
}
