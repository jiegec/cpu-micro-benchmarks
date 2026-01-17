#include "include/utils.h"

// ref:
// https://github.com/clamchowder/Microbenchmarks/blob/master/AsmGen/tests/LdqTest.cs
// https://github.com/clamchowder/Microbenchmarks/blob/master/AsmGen/tests/StqTest.cs
// https://github.com/clamchowder/Microbenchmarks/blob/master/AsmGen/tests/LoadSchedTest.cs
// https://dougallj.wordpress.com/2021/04/08/apple-m1-load-and-store-queue-measurements/
// https://gist.github.com/dougallj/0d4972967c625852956fcfe427b2054c
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);
  int min_size = 0;
  int max_size = 300;
  int num_dependent_patterns = 13;
  int num_patterns = num_dependent_patterns * 2;
  int sqrt_count = 20;
  int div_count = 40;
  // args: buffer1, buffer2, loop count
  fprintf(fp, ".text\n");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      fprintf(fp, ".global sched_size_%d_%d\n", size, pattern);
      fprintf(fp, ".align 4\n");
      fprintf(fp, "sched_size_%d_%d:\n", size, pattern);

      // pattern 0: dependent load
      // pattern 1: dependent store address
      // pattern 2: dependent store data
      // pattern 3: dependent alu
      // pattern 4: dependent imul
      // pattern 5: dependent fp
      // pattern 6: dependent crc
      // pattern 7: dependent idiv
      // pattern 8: dependent bfm(bitfield move)
      // pattern 9: dependent fjcvtzs
      // pattern 10: dependent f2i
      // pattern 11: dependent csel
      // pattern 12: dependent mrs nzcv
      // other patterns: dependent + independent
      int inst_pattern = pattern % num_dependent_patterns;
      bool has_independent = pattern >= num_dependent_patterns;
#if defined(HOST_AMD64)
      // use load for fp, alu, crc and imul, otherwise fp sqrt
      bool long_latency_load = (inst_pattern == 3) || (inst_pattern == 4) ||
                               (inst_pattern == 5) || (inst_pattern == 6) ||
                               (inst_pattern == 10);
#elif defined(HOST_AARCH64)
      // use int div for fp, otherwise fp sqrt
      bool long_latency_div =
          (inst_pattern == 5) || (inst_pattern == 9) || (inst_pattern == 10);
#endif
      bool has_branch = true;

#if defined(HOST_AMD64)
      fprintf(fp, "\tpushq %%r12\n");
      fprintf(fp, "\tpushq %%r13\n");
      fprintf(fp, "\tpushq %%r14\n");
      fprintf(fp, "\tpushq %%r15\n");
      fprintf(fp, "\tpushq %%rbx\n");

      // read two chains
      fprintf(fp, "\tmovq (%%rdi), %%rbx\n");
      fprintf(fp, "\tmovq (%%rsi), %%rcx\n");

      // initialize xmm0=1.0
      fprintf(fp, "\tmovq $1, %%r12\n");
      fprintf(fp, "\tcvtsi2sd %%r12, %%xmm0\n");

      fprintf(fp, "\t1:\n");

      // first long latency load
      if (long_latency_load) {
        fprintf(fp, "\tmovq (%%rbx), %%rbx\n");
        if (has_branch) {
          fprintf(fp, "\ttest %%rbx, %%rbx\n");
          fprintf(fp, "\tjz 2f\n");
        }
      } else {
        for (int j = 0; j < sqrt_count; j++) {
          fprintf(fp, "\tvsqrtsd %%xmm0, %%xmm0, %%xmm0\n");
        }
        if (has_branch) {
          fprintf(fp, "\tmovd %%xmm0, %%rax\n");
          fprintf(fp, "\ttest %%rax, %%rax\n");
          fprintf(fp, "\tjz 2f\n");
        }
      }

      // intermediate loads
      // we don't need to minus two for loads: the first load has already gone
      // out of scheduler
      for (int j = 0; j < max_size; j++) {
        if (j == 0) {
          if (long_latency_load) {
            // xmm0 = rbx
            fprintf(fp, "\tcvtsi2sd %%rbx, %%xmm0\n");
          } else {
            // rax = xmm0 = 1
            fprintf(fp, "\tcvtsd2si %%xmm0, %%rax\n");
          }
        }

        if (inst_pattern == 0) {
          // load
          if (j < size) {
            // r8-r11
            // read from stack with dependent offset
            fprintf(fp, "\tmov (%%rsp, %%rax, 8), %%r%d\n", 8 + (j % 4));
          } else if (has_independent) {
            // r12-r15
            fprintf(fp, "\tmov (%%rsi), %%r%d\n", 12 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent load
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 1) {
          // store address
          if (j < size) {
            // r8-r11
            // write to stack with dependent offset
            fprintf(fp, "\tmov %%r%d, %d(%%rsp, %%rax, 8)\n", 8 + (j % 4),
                    -0x40 - 8 * (j + 1));
          } else if (has_independent) {
            // r12-r15
            fprintf(fp, "\tmov %%r%d, %d(%%rsp)\n", 12 + (j % 4),
                    -0x40 - 8 * (j + 1));
            if (j == size + 32) {
              // do not add too many independent store
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 2) {
          // store data
          if (j < size) {
            // r8-r11
            // write to stack with dependent data
            fprintf(fp, "\tmov %%rax, %d(%%rsp)\n", -0x40 - 8 * (j + 1));
          } else if (has_independent) {
            // r12-r15
            fprintf(fp, "\tmov %%r%d, %d(%%rsp)\n", 12 + (j % 4),
                    -0x40 - 8 * (j + 1));
            if (j == size + 32) {
              // do not add too many independent store
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 3) {
          // alu
          if (j < size) {
            // r8-r11
            fprintf(fp, "\tadd %%rbx, %%r%d\n", 8 + (j % 4));
          } else if (has_independent) {
            // r12-r15
            fprintf(fp, "\tadd %%rdi, %%r%d\n", 12 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent add
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 4) {
          // imul
          if (j < size) {
            // r8-r11
            fprintf(fp, "\timul %%rbx, %%r%d\n", 8 + (j % 4));
          } else if (has_independent) {
            // r12-r15
            fprintf(fp, "\timul %%rdi, %%r%d\n", 12 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent imul
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 5) {
          // fp
          if (j < size) {
            // xmm8-xmm11
            fprintf(fp, "\taddss %%xmm0, %%xmm%d\n", 8 + (j % 4));
          } else if (has_independent) {
            // xmm12-xmm15
            fprintf(fp, "\taddss %%xmm4, %%xmm%d\n", 12 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent fp
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 6) {
          // crc
          if (j < size) {
            // r8-r11
            fprintf(fp, "\tcrc32 %%rbx, %%r%d\n", 8 + (j % 4));
          } else if (has_independent) {
            // r12-r15
            fprintf(fp, "\tcrc32 %%rdi, %%r%d\n", 12 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent crc32
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 10) {
          // f2i
          if (j < size) {
            // r8-r11
            fprintf(fp, "\tcvtsd2si %%xmm0, %%r%d\n", 8 + (j % 4));
          } else if (has_independent) {
            // r12-r15
            fprintf(fp, "\tcvtsd2si %%xmm4, %%r%d\n", 12 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent f2i
              // otherwise the performance difference is not evident
              break;
            }
          }
        }
      }

      if (long_latency_load) {
        // last long latency load
        fprintf(fp, "\tmovq (%%rcx), %%rcx\n");
      } else {
        // last long latency sqrt
        for (int j = 0; j < sqrt_count; j++) {
          fprintf(fp, "\tvsqrtsd %%xmm1, %%xmm1, %%xmm1\n");
        }
      }
      fprintf(fp, "\t2:\n");

      // forbid further speculation
      fprintf(fp, "\tlfence\n");
      fprintf(fp, "\tmfence\n");

      fprintf(fp, "\tsubl $1, %%edx\n");
      fprintf(fp, "\tjne 1b\n");

      // save two chains
      fprintf(fp, "\tmovq %%rbx, (%%rdi)\n");
      fprintf(fp, "\tmovq %%rcx, (%%rsi)\n");

      // return
      fprintf(fp, "\tpopq %%rbx\n");
      fprintf(fp, "\tpopq %%r15\n");
      fprintf(fp, "\tpopq %%r14\n");
      fprintf(fp, "\tpopq %%r13\n");
      fprintf(fp, "\tpopq %%r12\n");
      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      // leave some space for store test
      fprintf(fp, "\tsub sp, sp, #0x100\n");
      fprintf(fp, ".arch armv8.3-a\n");
      // read two chains
      fprintf(fp, "\tldr x3, [x0]\n");
      fprintf(fp, "\tldr x4, [x1]\n");

      // initialize d0, x6 and x7
      fprintf(fp, "\tfmov d0, #1.0\n");
      fprintf(fp, "\tmov x6, #2\n");
      fprintf(fp, "\tmov x7, #2\n");

      fprintf(fp, "\t1:\n");

      // first long latency div
      if (long_latency_div) {
        for (int j = 0; j < div_count; j++) {
          fprintf(fp, "\tsdiv x6, x3, x6\n");
        }
        // long latency load
        // fprintf(fp, "\tldr x3, [x3]\n");
        if (has_branch) {
          fprintf(fp, "\tcbz x6, 2f\n");
        }
      } else {
        for (int j = 0; j < sqrt_count; j++) {
          fprintf(fp, "\tfsqrt d0, d0\n");
        }
        if (has_branch) {
          fprintf(fp, "\tfmov x5, d0\n");
          fprintf(fp, "\tcbz x5, 2f\n");
        }
      }

      // intermediate loads
      // we don't need to minus two for loads: the first load has already gone
      // out of scheduler
      for (int j = 0; j < max_size; j++) {
        if (j == 0) {
          if (long_latency_div) {
            // d0 = x6
            fprintf(fp, "\tfmov d0, x6\n");
          } else {
            // x5 = d0 = 1
            fprintf(fp, "\tfcvtns x5, d0\n");
          }
        }
        if (inst_pattern == 0) {
          // load
          if (j < size) {
            // x9-x12
            // read from stack with dependent offset
            fprintf(fp, "\tldr x%d, [sp, x5, lsl 3]\n", 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tldr x%d, [sp, xzr, lsl 3]\n", 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent loads
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 1) {
          // store address
          if (j < size) {
            // x9-x12
            // write to stack with dependent offset
            fprintf(fp, "\tstr x%d, [sp, x5, lsl 3]\n", 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tstr x%d, [sp, xzr, lsl 3]\n", 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent stores
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 2) {
          // store data
          if (j < size) {
            // write to stack with dependent offset
            fprintf(fp, "\tstr x5, [sp]\n");
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tstr x%d, [sp, xzr, lsl 3]\n", 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent stores
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 3) {
          // alu
          // 3 ops ahead shares scheduler with alu: fmov + cbz + fcvtns
          if (j < size - 3) {
            // x9-x12
            fprintf(fp, "\tadd x%d, x%d, x5\n", 9 + (j % 4), 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tadd x%d, x%d, x4\n", 13 + (j % 4), 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent adds
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 4) {
          // imul
          if (j < size) {
            // x9-x12
            fprintf(fp, "\tmul x%d, x%d, x5\n", 9 + (j % 4), 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tmul x%d, x%d, x4\n", 13 + (j % 4), 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent muls
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 5) {
          // fp
          if (j < size) {
            // d16-d19
            fprintf(fp, "\tfadd d%d, d%d, d0\n", 16 + (j % 4), 16 + (j % 4));
          } else if (has_independent) {
            // d20-d23
            fprintf(fp, "\tfadd d%d, d%d, d4\n", 20 + (j % 4), 20 + (j % 4));
          }
        } else if (inst_pattern == 6) {
          // crc
          if (j < size) {
            // x9-x12
            fprintf(fp, "\tcrc32b w%d, w%d, w5\n", 9 + (j % 4), 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tcrc32b w%d, w%d, w4\n", 13 + (j % 4), 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent crc32b
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 7) {
          // idiv
          if (j < size) {
            // x9-x12
            fprintf(fp, "\tudiv w%d, w%d, w5\n", 9 + (j % 4), 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tudiv w%d, w%d, w4\n", 13 + (j % 4), 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent udiv
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 8) {
          // bfm
          if (j < size) {
            // x9-x12
            fprintf(fp, "\tbfm w%d, w5, #5, #1\n", 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tbfm w%d, w4, #5, #1\n", 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent ubfm
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 9) {
#ifndef NO_FJCVTZS
          // fjcvtzs
          if (j < size) {
            // x9-x12
            fprintf(fp, "\tfjcvtzs w%d, d0\n", 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tfjcvtzs w%d, d4\n", 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent fjcvtzs
              // otherwise the performance difference is not evident
              break;
            }
          }
#endif
        } else if (inst_pattern == 10) {
          // fmov
          if (j < size) {
            // x9-x12
            fprintf(fp, "\tfmov x%d, d0\n", 9 + (j % 4));
          } else if (has_independent) {
            // x13-x16
            fprintf(fp, "\tfmov x%d, d4\n", 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent fmov
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 11) {
          // csel
          if (j < size) {
            if (j == 0) {
              fprintf(fp, "\tcmp w5, wzr\n");
            }
            // x9-x12
            fprintf(fp, "\tcsel w%d, w5, w%d, eq\n", 9 + (j % 4), 9 + (j % 4));
          } else if (has_independent) {
            if (j == size) {
              fprintf(fp, "\tcmp w4, wzr\n");
            }
            // x13-x16
            fprintf(fp, "\tcsel w%d, w4, w%d, eq\n", 13 + (j % 4),
                    13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent ubfm
              // otherwise the performance difference is not evident
              break;
            }
          }
        } else if (inst_pattern == 12) {
          // mrs nzcv
          if (j < size) {
            if (j == 0) {
              fprintf(fp, "\tcmp w5, wzr\n");
            }
            // x9-x12
            fprintf(fp, "\tmrs x%d, nzcv\n", 9 + (j % 4));
          } else if (has_independent) {
            if (j == size) {
              fprintf(fp, "\tcmp w4, wzr\n");
            }
            // x13-x16
            fprintf(fp, "\tmrs x%d, nzcv\n", 13 + (j % 4));
            if (j == size + 32) {
              // do not add too many independent ubfm
              // otherwise the performance difference is not evident
              break;
            }
          }
        }
      }

      if (long_latency_div) {
        // last long latency div
        for (int j = 0; j < div_count; j++) {
          fprintf(fp, "\tsdiv x7, x3, x7\n");
        }
        // long latency load
        // fprintf(fp, "\tldr x4, [x4]\n");
      } else {
        // last long latency sqrt
        for (int j = 0; j < sqrt_count; j++) {
          fprintf(fp, "\tfsqrt d1, d1\n");
        }
      }
      fprintf(fp, "\t2:\n");

      // forbid further speculation
      fprintf(fp, "\tdsb ish\n");
      fprintf(fp, "\tisb\n");

      fprintf(fp, "\tsubs x2, x2, #1\n");
      fprintf(fp, "\tbne 1b\n");

      // save two chains
      fprintf(fp, "\tstr x3, [x0]\n");
      fprintf(fp, "\tstr x4, [x1]\n");

      fprintf(fp, "\tadd sp, sp, #0x100\n");
      fprintf(fp, "\tret\n");
#endif
    }
  }

  define_gadgets_array(fp, "sched_size_gadgets");
  for (int pattern = 0; pattern < num_patterns; pattern++) {
    for (int size = min_size; size <= max_size; size++) {
      add_gadget(fp, "sched_size_%d_%d", size, pattern);
    }
  }
  return 0;
}
