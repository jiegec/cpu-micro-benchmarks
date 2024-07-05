#include <assert.h>
#include <set>
#include <stdio.h>
#include <string.h>
FILE *fp;

// generate gadget for rob test
void gen_rob_gadget() {
  int repeat = 20;
  int min_size = 32;
  int max_size = 384;
  // args: loop count, buffer
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    fprintf(fp, ".global rob_size_%d\n", size);
    fprintf(fp, ".align 4\n");
    fprintf(fp, "rob_size_%d:\n", size);
#ifdef __aarch64__
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tldr x0, [x0]\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
    }
    fprintf(fp, "\tsubs x1, x1, #1\n");
    fprintf(fp, "\tbne 1b\n");
    fprintf(fp, "\tret\n");
#elif defined(__x86_64__)
    fprintf(fp, "\tmovq %%rdi, %%r8\n");
    fprintf(fp, "\tmovq %%rsi, %%rax\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tmovq (%%r8), %%r8\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
    }
    fprintf(fp, "\tsubl $1, %%eax\n");
    fprintf(fp, "\tjne 1b\n");
    fprintf(fp, "\tmovq %%r8, %%rax\n");
    fprintf(fp, "\tret\n");
#elif defined(__powerpc__)
    fprintf(fp, "\tmtctr 4\n");
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tld 3, 0(3)\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
    }
    fprintf(fp, "\tbdnz 1b\n");
    fprintf(fp, "\tblr\n");
#elif defined(__loongarch__)
    fprintf(fp, "\t1:\n");
    for (int i = 0; i < repeat; i++) {
      fprintf(fp, "\tld.d $a0, $a0, 0\n");
      for (int j = 0; j < size - 1; j++) {
        fprintf(fp, "\tnop\n");
      }
    }
    fprintf(fp, "\taddi.d $a1, $a1, -1\n");
    fprintf(fp, "\tbne $a1, $zero, 1b\n");
    fprintf(fp, "\tret\n");
#endif
  }

  fprintf(fp, ".data\n");
  // for macOS
  fprintf(fp, ".global _rob_gadgets\n");
  fprintf(fp, "_rob_gadgets:\n");
  fprintf(fp, ".global rob_gadgets\n");
  fprintf(fp, "rob_gadgets:\n");
  for (int size = min_size; size <= max_size; size++) {
#ifdef __aarch64__
    fprintf(fp, ".dword rob_size_%d\n", size);
#elif defined(__x86_64__)
    fprintf(fp, ".dc.a rob_size_%d\n", size);
#elif defined(__powerpc__)
    fprintf(fp, ".dc.a rob_size_%d\n", size);
#elif defined(__loongarch__)
    fprintf(fp, ".dc.a rob_size_%d\n", size);
#endif
  }
}

// generate gadget for btb test
// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/tests/BtbTest.cs
void gen_btb_gadget() {
  int min_size = 4;
  int max_product = 262144;
  int min_stride = 4;
  int max_stride = 65536;

  // args: loop count
  fprintf(fp, ".text\n");
  for (int stride = min_stride; stride <= max_stride; stride *= 2) {
    std::set<int> sizes;
    for (int size_base = min_size; size_base <= max_product / stride;
         size_base *= 2) {
      for (int size_mid = size_base; size_mid < size_base * 2;
           size_mid *= 1.25992105) {
        for (int size = size_mid - 1; size <= size_mid + 1; size++) {
          if (sizes.find(size) != sizes.end()) {
            continue;
          }
          sizes.insert(size);

          fprintf(fp, ".global btb_size_%d_%d\n", size, stride);
          fprintf(fp, ".balign 32\n");
          fprintf(fp, "btb_size_%d_%d:\n", size, stride);
#ifdef __aarch64__
          fprintf(fp, "\t1:\n");
          // the last loop is bne 1b
          for (int i = 0; i < size - 1; i++) {
            fprintf(fp, "\tb 2f\n");
            // fill nops so that branch instructions have the specified stride
            fprintf(fp, "\t.balign %d\n", stride);
            fprintf(fp, "\t2:\n");
          }
          fprintf(fp, "\tsubs x0, x0, #1\n");
          fprintf(fp, "\tbne 1b\n");
          fprintf(fp, "\tret\n");
#elif defined(__x86_64__)
          fprintf(fp, "\t1:\n");
          // the last loop is bne 1b
          for (int i = 0; i < size - 1; i++) {
            fprintf(fp, "\tjmp 2f\n");
            // fill nops so that branch instructions have the specified stride
            fprintf(fp, "\t.balign %d\n", stride);
            fprintf(fp, "\t2:\n");
          }
          fprintf(fp, "\tdec %%rdi\n");
          fprintf(fp, "\tjne 1b\n");
          fprintf(fp, "\tret\n");
#endif
        }
      }
    }
  }

  fprintf(fp, ".data\n");
  // for macOS
  fprintf(fp, ".global _btb_gadgets\n");
  fprintf(fp, "_btb_gadgets:\n");
  fprintf(fp, ".global btb_gadgets\n");
  fprintf(fp, "btb_gadgets:\n");
  for (int stride = min_stride; stride <= max_stride; stride *= 2) {
    std::set<int> sizes;
    for (int size_base = min_size; size_base <= max_product / stride;
         size_base *= 2) {
      for (int size_mid = size_base; size_mid < size_base * 2;
           size_mid *= 1.25992105) {
        for (int size = size_mid - 1; size <= size_mid + 1; size++) {
          if (sizes.find(size) != sizes.end()) {
            continue;
          }
          sizes.insert(size);

#ifdef __aarch64__
          fprintf(fp, ".dword btb_size_%d_%d\n", size, stride);
#elif defined(__x86_64__)
          fprintf(fp, ".dc.a btb_size_%d_%d\n", size, stride);
#endif
        }
      }
    }
  }
}

// generate gadget for ras test
// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/tests/ReturnStackTest.cs
void gen_ras_gadget() {
  int min_size = 1;
  int max_size = 64;

  // args: loop count
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    // entry
    fprintf(fp, ".global ras_size_%d\n", size);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "ras_size_%d:\n", size);
#ifdef __aarch64__
    // save lr
    fprintf(fp, "\tsub sp, sp, #0x20\n");
    fprintf(fp, "\tstp x29, x30, [sp, #0x10]\n");

    fprintf(fp, "\t1:\n");
    // call function
    fprintf(fp, "\tbl ras_func_%d\n", size - 1);
    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    // restore lr
    fprintf(fp, "\tldp x29, x30, [sp, #0x10]\n");
    fprintf(fp, "\tadd sp, sp, #0x20\n");
    fprintf(fp, "\tret\n");
#elif defined(__x86_64__)
    fprintf(fp, "\t1:\n");
    // call function
    fprintf(fp, "\tcall ras_func_%d\n", size - 1);
    fprintf(fp, "\tdec %%rdi\n");
    fprintf(fp, "\tjne 1b\n");
    fprintf(fp, "\tret\n");
#endif

    // inner function
    fprintf(fp, ".global ras_func_%d\n", size);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "ras_func_%d:\n", size);

#ifdef __aarch64__
    // save lr
    fprintf(fp, "\tsub sp, sp, #0x20\n");
    fprintf(fp, "\tstp x29, x30, [sp, #0x10]\n");

    // call lower function
    fprintf(fp, "\tbl ras_func_%d\n", size - 1);

    // restore lr
    fprintf(fp, "\tldp x29, x30, [sp, #0x10]\n");
    fprintf(fp, "\tadd sp, sp, #0x20\n");
    fprintf(fp, "\tret\n");
#elif defined(__x86_64__)
    fprintf(fp, "\tcall ras_func_%d\n", size - 1);
    fprintf(fp, "\tret\n");
#endif
  }

  // recursion base
  fprintf(fp, ".global ras_func_%d\n", 0);
  fprintf(fp, ".balign 32\n");
  fprintf(fp, "ras_func_%d:\n", 0);
  fprintf(fp, "\tret\n");

  fprintf(fp, ".data\n");
  // for macOS
  fprintf(fp, ".global _ras_gadgets\n");
  fprintf(fp, "_ras_gadgets:\n");
  fprintf(fp, ".global ras_gadgets\n");
  fprintf(fp, "ras_gadgets:\n");
  for (int size = min_size; size <= max_size; size++) {
#ifdef __aarch64__
    fprintf(fp, ".dword ras_size_%d\n", size);
#elif defined(__x86_64__)
    fprintf(fp, ".dc.a ras_size_%d\n", size);
#endif
  }
}

// generate gadget for bp test
// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/tests/BranchHistoryTest.cs
// https://github.com/ChipsandCheese/Microbenchmarks/blob/master/AsmGen/DataFiles/GccBranchHistFunction.c
void gen_bp_gadget() {
  int min_size = 1;
  int max_size = 1024;

  // args: loop count, pattern array[branch][history], history len
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size *= 2) {
    // entry
    fprintf(fp, ".global bp_size_%d\n", size);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "bp_size_%d:\n", size);
#ifdef __aarch64__
    // save registers
    fprintf(fp, "\tsub sp, sp, #0x40\n");
    fprintf(fp, "\tstp x11, x12, [sp, #0x30]\n");
    fprintf(fp, "\tstp x15, x16, [sp, #0x20]\n");
    fprintf(fp, "\tstp x13, x14, [sp, #0x10]\n");
    fprintf(fp, "\teor x16, x16, x16\n");
    fprintf(fp, "\teor x15, x15, x15\n");
    fprintf(fp, "\teor x12, x12, x12\n");
    fprintf(fp, "\teor x11, x11, x11\n");

    fprintf(fp, "\t1:\n");
    fprintf(fp, "\teor x14, x14, x14\n");

    // data-dependent branches
    // w14: branch index
    // w16: history index
    for (int i = 0; i < size; i++) {
      fprintf(fp, "\tldr x15, [x1, w14, uxtw #3]\n");
      fprintf(fp, "\tadd w14, w14, 1\n");
      fprintf(fp, "\tldr w13, [x15, w16, uxtw #2]\n");
      // conditional branch
      fprintf(fp, "\tcbnz x13, 2f\n");
      fprintf(fp, "\tadd x12, x12, 1\n");
      fprintf(fp, "\t2:\n");
    }

    // increment w16, set w16 to 0 if w16 == history len
    fprintf(fp, "\tadd w16, w16, 1\n");
    fprintf(fp, "\tcmp w16, w2\n");
    fprintf(fp, "\tcsel w16, w11, w16, EQ\n");

    // loop
    fprintf(fp, "\tsub x0, x0, 1\n");
    fprintf(fp, "\tcbnz x0, 1b\n");

    // restore regs
    fprintf(fp, "\tldp x11, x12, [sp, #0x30]\n");
    fprintf(fp, "\tldp x15, x16, [sp, #0x20]\n");
    fprintf(fp, "\tldp x13, x14, [sp, #0x10]\n");
    fprintf(fp, "\tadd sp, sp, #0x40\n");
    fprintf(fp, "\tret\n");
#endif
  }

  fprintf(fp, ".data\n");
  // for macOS
  fprintf(fp, ".global _bp_gadgets\n");
  fprintf(fp, "_bp_gadgets:\n");
  fprintf(fp, ".global bp_gadgets\n");
  fprintf(fp, "bp_gadgets:\n");
  for (int size = min_size; size <= max_size; size *= 2) {
#ifdef __aarch64__
    fprintf(fp, ".dword bp_size_%d\n", size);
#endif
  }
}

// generate gadget for ghr test
// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf
void gen_ghr_gadget() {
  int min_size = 1;
  int max_size = 128;

  // args: loop count, random array
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size++) {
    fprintf(fp, ".global ghr_size_%d\n", size);
    fprintf(fp, ".balign 32\n");
    fprintf(fp, "ghr_size_%d:\n", size);
#ifdef __aarch64__
    // save registers
    fprintf(fp, "\tsub sp, sp, #0x20\n");
    fprintf(fp, "\tstp x11, x12, [sp, #0x10]\n");

    fprintf(fp, "\t1:\n");

    // first branch based on random value
    fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");
    fprintf(fp, "\tcbnz w11, 2f\n");
    fprintf(fp, "\t2:\n");

    // the first branch is cbnz w11, 2f
    // forward always-taken branches
    for (int i = 0; i < size - 1; i++) {
      fprintf(fp, "\tb 2f\n");
      fprintf(fp, "\t2:\n");
    }

    // last branch based on the same random value
    fprintf(fp, "\tcbnz w11, 2f\n");
    fprintf(fp, "\t2:\n");

    fprintf(fp, "\tsubs x0, x0, #1\n");
    fprintf(fp, "\tbne 1b\n");

    // restore regs
    fprintf(fp, "\tldp x11, x12, [sp, #0x10]\n");
    fprintf(fp, "\tadd sp, sp, #0x20\n");
    fprintf(fp, "\tret\n");
#endif
  }

  fprintf(fp, ".data\n");
  // for macOS
  fprintf(fp, ".global _ghr_gadgets\n");
  fprintf(fp, "_ghr_gadgets:\n");
  fprintf(fp, ".global ghr_gadgets\n");
  fprintf(fp, "ghr_gadgets:\n");
  for (int size = min_size; size <= max_size; size++) {
#ifdef __aarch64__
    fprintf(fp, ".dword ghr_size_%d\n", size);
#endif
  }
}

int main(int argc, char *argv[]) {
  assert(argc == 3);
  fp = fopen(argv[2], "w");
  assert(fp);
  if (strcmp(argv[1], "rob") == 0) {
    gen_rob_gadget();
  } else if (strcmp(argv[1], "btb") == 0) {
    gen_btb_gadget();
  } else if (strcmp(argv[1], "ras") == 0) {
    gen_ras_gadget();
  } else if (strcmp(argv[1], "bp") == 0) {
    gen_bp_gadget();
  } else if (strcmp(argv[1], "ghr") == 0) {
    gen_ghr_gadget();
  } else {
    fprintf(stderr, "Unknown gadget to generate!\n");
    return 1;
  }
  fclose(fp);
  return 0;
}
