#include <assert.h>
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
void gen_btb_gadget() {
  int min_size = 4;
  int max_size = 8192;
  int min_stride = 4;
  int max_stride = 64;

  // args: loop count
  fprintf(fp, ".text\n");
  for (int size = min_size; size <= max_size; size *= 2) {
    for (int stride = min_stride; stride <= max_stride; stride *= 2) {
      fprintf(fp, ".global btb_size_%d_%d\n", size, stride);
      fprintf(fp, ".align 4\n");
      fprintf(fp, "btb_size_%d_%d:\n", size, stride);
#ifdef __aarch64__
      fprintf(fp, "\t1:\n");
      // the last loop is bne 1b
      for (int i = 0; i < size - 1; i++) {
        fprintf(fp, "\tb 2f\n");
        // fill nops so that branch instructions have the specified stride
        for (int i = 0;i < (stride / 4) - 1;i++) {
          fprintf(fp, "\tnop\n");
        }
        fprintf(fp, "\t2:\n");
      }
      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tbne 1b\n");
      fprintf(fp, "\tret\n");
#endif
    }
  }

  fprintf(fp, ".data\n");
  // for macOS
  fprintf(fp, ".global _btb_gadgets\n");
  fprintf(fp, "_btb_gadgets:\n");
  fprintf(fp, ".global btb_gadgets\n");
  fprintf(fp, "btb_gadgets:\n");
  for (int size = min_size; size <= max_size; size *= 2) {
    for (int stride = min_stride; stride <= max_stride; stride *= 2) {
#ifdef __aarch64__
      fprintf(fp, ".dword btb_size_%d_%d\n", size, stride);
#endif
    }
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
  } else {
    fprintf(stderr, "Unknown gadget to generate!\n");
    return 1;
  }
  fclose(fp);
  return 0;
}
