#include "include/jit.h"
#include "include/utils.h"
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

// check index bit conflict xor PHR vs PHR
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);

#ifdef HOST_AARCH64
// inject w11[bit] to phrt LSB
// does not clobber registers
void inject_w11_phrt(jit *jit_main, int bit) {
  // stp x12, x13, [sp, #0]
  jit_main->stp64(12, 13, 31, 0);
  jit_main->align(64, 0);
  // ensure first_target_1 xor first_target_2 == 0x4
  uint8_t *first_target_1 = jit_main->get_cur() + 128;
  uint8_t *first_target_2 = first_target_1 + 4;
  assert(((size_t)first_target_1 ^ (size_t)first_target_2) == 0x4);
  // x12 = first_target_1
  jit_main->li64(12, (uint64_t)first_target_1);
  // x13 = x11 & (1 << bit)
  jit_main->li64(13, (1 << bit));
  jit_main->and64(13, 11, 13);
  // cmp w13, 0
  jit_main->cmp32(13, 0);
  // x13 = first_target_2
  jit_main->li64(13, (uint64_t)first_target_2);
  // csel x12, x12, x13, eq
  jit_main->csel64(12, 12, 13, jit::COND_EQ);
  // br x12
  jit_main->br64(12);
  jit_main->set_cur(first_target_1);
  jit_main->nop();
  // ldp x12, x13, [sp, #0]
  jit_main->ldp64(12, 13, 31, 0);
}

// inject w11[bit] to phrb LSB
// does not clobber registers if jump_x20 is false
// clobber x12, x13 if jump_x20 is true
void inject_w11_phrb(jit *jit_main, int bit, bool jump_x20) {
  // inject using 3 branches
  // 1. same B, T[2] differs
  // 2. B[2] differs, T[3] differs
  // 3. B[2] & B[3] differs, same T

  // stp x12, x13, [sp, #0]
  jit_main->stp64(12, 13, 31, 0);
  jit_main->align(64, 0);
  // ensure first_target_1 xor first_target_2 == 0x4
  uint8_t *first_target_1 = jit_main->get_cur() + 128;
  uint8_t *first_target_2 = first_target_1 + 4;
  assert(((size_t)first_target_1 ^ (size_t)first_target_2) == 0x4);
  // x12 = first_target_1
  jit_main->li64(12, (uint64_t)first_target_1);
  // x13 = x11 & (1 << bit)
  jit_main->mov64(13, (1 << bit));
  jit_main->and64(13, 11, 13);
  // cmp w13, 0
  jit_main->cmp32(13, 0);
  // x13 = first_target_2
  jit_main->li64(13, (uint64_t)first_target_2);
  // csel x12, x12, x13, eq
  jit_main->csel64(12, 12, 13, jit::COND_EQ);
  // br x12
  jit_main->br64(12);
  jit_main->set_cur(first_target_1);
  jit_main->nop();

  jit_main->align(64, 0);
  // ensure first_target_3 xor first_target_4 == 0x8
  uint8_t *first_target_3 = jit_main->get_cur() + 128;
  uint8_t *first_target_4 = first_target_3 + 8;
  assert(((size_t)first_target_3 ^ (size_t)first_target_4) == 0x8);

  jit_main->set_cur(first_target_1);
  jit_main->b(first_target_3);
  jit_main->set_cur(first_target_2);
  jit_main->b(first_target_4);

  uint8_t *first_target_5 = first_target_3 + 128;
  jit_main->set_cur(first_target_3);
  if (jump_x20) {
    jit_main->br64(20);
  } else {
    jit_main->b(first_target_5);
  }
  jit_main->nop();
  jit_main->set_cur(first_target_4);
  jit_main->nop();
  if (jump_x20) {
    jit_main->br64(20);
  } else {
    jit_main->b(first_target_5);
  }
  jit_main->set_cur(first_target_5);
  // ldp x12, x13, [sp, #0]
  jit_main->ldp64(12, 13, 31, 0);
}
#endif

struct phr_injection {
  // which PHR bit to inject?
  int phr_bit;
  // which bit from w11 to inject?
  int k_bit;
  // inject target?
  bool inject_target;

  // order by phr_bit
  bool operator<(const struct phr_injection &other) const {
    return phr_bit < other.phr_bit;
  }
};

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  int min_branches = 1;
  int max_branches = 8;
  int min_dummy_branches = 0;
  int max_dummy_branches = PHR_BRANCHES;
  // test PHRT or PHRB in first bit
  bool inject_target_first = true;
  // test PHRT or PHRB
  bool inject_target = true;
  uintptr_t target_branches[64];

  int opt;
  // which table to test?
  int test_table = 1;
  while ((opt = getopt(argc, argv, "t:bB")) != -1) {
    switch (opt) {
    case 't':
      sscanf(optarg, "%d", &test_table);
      break;
    case 'b':
      inject_target = false;
      break;
    case 'B':
      inject_target_first = false;
      break;
    default:
      fprintf(stderr, "Usage: %s [-t table] [-b] [-B]\n", argv[0]);
      fprintf(stderr,
              "\t-t table: which table to test? 1 for longest, 2 for 2nd\n");
      fprintf(stderr, "\t-b: test phrb instead of phrt in second bit\n");
      fprintf(stderr, "\t-B: test phrb instead of phrt in first bit\n");
      exit(EXIT_FAILURE);
    }
  }
  printf("Test table #%d\n", test_table);
  printf("Injecting %s vs %s\n", inject_target_first ? "PHRT" : "PHRB",
         inject_target ? "PHRT" : "PHRB");

  bind_to_core();
  setup_perf_cond_branch_misses();
  FILE *fp = fopen("pht_index_bits_xor_phr.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  std::vector<int> additional_phr_bits;
  int history_bits = -1;

#ifdef QUALCOMM_ORYON
  if (test_table == 1) {
    // longest table
    additional_phr_bits = {54, 60, 65, 70, 75, 80, 85, 90, 95};
  } else if (test_table == 2) {
    // second longest tage
    additional_phr_bits = {28, 31, 34, 38, 41, 44, 48, 51};
    history_bits = 52;
  } else if (test_table == 3) {
    // third longest tage
    additional_phr_bits = {15, 17, 19, 21, 22, 24, 26};
    history_bits = 27;
  } else if (test_table == 4) {
    // fourth longest tage
    additional_phr_bits = {7, 8, 9, 10, 11, 12, 13};
    history_bits = 14;
  } else if (test_table == 5) {
    // fifth longest tage
    additional_phr_bits = {4, 5, 6};
    history_bits = 7;
  } else if (test_table == 6) {
    // sixth longest tage
    additional_phr_bits = {0, 1, 2, 3};
    history_bits = 4;
  }
#else
  // index bits of firestorm tage
  if (test_table == 1) {
    // longest table
    additional_phr_bits = {58, 63, 68, 73, 78, 83, 88, 93, 99};
  } else if (test_table == 2) {
    // second longest tage
    additional_phr_bits = {32, 35, 38, 42, 45, 49, 52, 56};
    history_bits = 57;
  } else if (test_table == 3) {
    // third longest tage
    additional_phr_bits = {18, 21, 23, 26, 28, 31};
    history_bits = 32;
  } else if (test_table == 4) {
    // fourth longest tage
    additional_phr_bits = {11, 12, 13, 14, 15, 16, 17};
    history_bits = 18;
  } else if (test_table == 5) {
    // fifth longest tage
    additional_phr_bits = {6, 7, 8, 9, 10};
    history_bits = 11;
    max_dummy_branches = 16;
  } else if (test_table == 6) {
    // sixth longest tage
    additional_phr_bits = {0, 1, 2, 3, 4, 5};
    history_bits = 11; // let's consider the two tables together
    max_branches = 16;
    max_dummy_branches = 16;
  }
#endif

  fprintf(fp, "phr,branches,dummy,min,avg,max\n");
  for (int additional_phr_bit : additional_phr_bits) {
    for (int branches = min_branches; branches <= max_branches; branches++) {
      for (int dummy_branches = min_dummy_branches;
           dummy_branches <= max_dummy_branches; dummy_branches++) {
        std::vector<double> history;
        int iterations = 100;
        history.reserve(iterations);

        // main part
        gadget entry = NULL;
        jit *jit_main = NULL;
        jit *jit_pages[64] = {NULL};
#ifdef HOST_AARCH64
        jit_main = new jit(16 * 1024);
        entry = (gadget)jit_main->get_cur();

        // save registers
        // sub sp, sp, #0x40
        jit_main->sub64(31, 31, 0x40);
        // stp x19, x20, [sp, #0x10]
        jit_main->stp64(19, 20, 31, 0x10);

        // loop_begin:
        uint8_t *loop_begin = jit_main->get_cur();

        // read random value to w11
        // ldr w11, [x1, w0, uxtw #2]
        jit_main->ldr32shift2(11, 1, 0);

        // shift phr
        for (int i = 0; i < 200; i++) {
          jit_main->b(jit_main->get_cur() + 4);
        }

        std::vector<phr_injection> injections;

        // fill random bits in longer tables
        if (history_bits != -1) {
          for (int i = 0; i < 15; i++) {
            injections.push_back(phr_injection{
                .phr_bit = history_bits + i,
                .k_bit = 7 + i,
                .inject_target = true,
            });
          }
        }

        // inject w11[1] to PHRT[additional_phr_bit]
        injections.push_back(phr_injection{
            .phr_bit = additional_phr_bit,
            .k_bit = 1,
            .inject_target = inject_target_first,
        });

        // compute branches address (saved in x20)
        // jump to branches randomly
        // x13 = x11 / 4, the lower two bits are used to inject k
        jit_main->lsr64(13, 11, 2);
        // x15 = branches
        // x14 = x13 / x15
        jit_main->mov64(15, branches);
        jit_main->udiv64(14, 13, 15);
        // x12 = x13 % x14
        jit_main->msub64(12, 14, 15, 13);
        // x16 = target_branches
        jit_main->li64(16, (uint64_t)&target_branches[0]);
        // x20 = x16[x12]
        jit_main->ldr64shift3(20, 16, 12);

        // inject w11[0] to PHRT[dummy_branches] or PHRB[dummy_branches]
        injections.push_back(phr_injection{
            .phr_bit = dummy_branches,
            .k_bit = 0,
            .inject_target = inject_target,
        });

        // handle injections above
        // 1. inject & shift
        // 2. jump to branches with T[31:2]=0 and same B
        // and we handle PHRT[0] & PHRB[0] specifically
        std::sort(injections.begin(), injections.end());
        for (int i = injections.size() - 1; i >= 0; i--) {
          int shift;
          if (i > 0) {
            shift = injections[i].phr_bit - injections[i - 1].phr_bit -
                    (injections[i - 1].inject_target ? 1 : 3);
          } else {
            shift = injections[i].phr_bit;
          }

          if (injections[i].inject_target) {
            if (i == 0) {
              // last injection?
              if (shift == 0) {
                // we can't inject PHRT[0] directly, since there is a branch
                // afterwards. so we inject PHRT[0] = w11[k_bit] in last branch.

                // we inject PHRT[0] = w11[k_bit] here
                // x10 = x11 & (1 << k_bit)
                jit_main->mov64(10, (1 << injections[i].k_bit));
                jit_main->and64(10, 11, 10);
                // x10 = x10 << 2
                jit_main->lsl64(10, 10, 2);
                // x20 = x20 + x10
                jit_main->addr64(20, 20, 10);
                // br x20
                jit_main->br64(20);
              } else {
                inject_w11_phrt(jit_main, injections[i].k_bit);

                // shift phr by dummy_branches-1 time
                for (int i = 0; i < shift - 1; i++) {
                  jit_main->b(jit_main->get_cur() + 4);
                }
                // br x20
                jit_main->br64(20);
              }
            } else {
              // normal injection
              inject_w11_phrt(jit_main, injections[i].k_bit);
              for (int j = 0; j < shift; j++) {
                jit_main->b(jit_main->get_cur() + 4);
              }
            }
          } else {
            // inject w11[k_bit] to phrb LSB
            if (i == 0) {
              if (shift == 0) {
                // last branch
                inject_w11_phrb(jit_main, injections[i].k_bit, true);
              } else {
                inject_w11_phrb(jit_main, injections[i].k_bit, false);

                // shift phr by dummy_branches-1 time
                for (int i = 0; i < shift - 1; i++) {
                  jit_main->b(jit_main->get_cur() + 4);
                }
                // br x20
                jit_main->br64(20);
              }
            } else {
              // normal injection
              inject_w11_phrb(jit_main, injections[i].k_bit, false);
              for (int j = 0; j < shift; j++) {
                jit_main->b(jit_main->get_cur() + 4);
              }
            }
          }
        }

        // after branches, jump back here
        // branch_end:
        uint8_t *branch_end = jit_main->get_cur();
        uint8_t *loop_end = jit_main->get_cur() + 128;
        // x12 = loop_begin
        jit_main->li64(12, (uint64_t)loop_begin);
        // x13 = loop_end
        jit_main->li64(13, (uint64_t)loop_end);
        // subs x0, x0, #1
        jit_main->subs64(0, 0, 1);
        // csel x12, x12, x13, ne
        jit_main->csel64(12, 12, 13, jit::COND_NE);
        // br x12
        jit_main->br64(12);

        // loop_end:
        jit_main->set_cur(loop_end);
        // retore regs
        jit_main->ldp64(19, 20, 31, 0x10);
        jit_main->add64(31, 31, 0x40);
        jit_main->ret();
        // jit_main->dump();

        // following Half & Half: use lower bits to differentiate
        // it does work by avoiding using pc bits in index
        int offsets[64];
        for (int i = 0; i < branches; i++) {
          offsets[i] = 4 * i;
        }

        for (int i = 0; i < branches; i++) {
          // align to 32-bit boundary
          // M1 firestorm does not use T[32+] in phr
          jit_pages[i] = new jit((void *)(0x100000000L * (i + 1)), 1024 * 1024);
          uint8_t *cur = jit_pages[i]->get_cur();
          target_branches[i] = (uintptr_t)cur;
          // add nops until offset
          for (int j = 0; j < offsets[i] / 4; j++) {
            jit_pages[i]->nop();
          }
          // add extra nop to handling dummy_branches == 0 case
          jit_pages[i]->nop();

          // branch on w11[0] xor w11[1]
          // x10 = (x11 & 2) >> 1
          jit_pages[i]->mov64(10, 2);
          jit_pages[i]->and64(10, 11, 10);
          jit_pages[i]->lsr64(10, 10, 1);
          // x12 = x11 & 1
          jit_pages[i]->mov64(12, 1);
          jit_pages[i]->and64(12, 11, 12);
          // x10 = x10 xor x12
          jit_pages[i]->eor64(10, 10, 12);
          jit_pages[i]->cbnz64(10, jit_pages[i]->get_cur() + 4);

          // jump to branch_end
          jit_pages[i]->li64(12, (uint64_t)branch_end);
          jit_pages[i]->br64(12);
          char name[256];
          sprintf(name, "jit_page_%d.bin", i);
          // jit_pages[i]->dump(name);

          jit_pages[i]->protect();
        }

        jit_main->protect();
#endif

        double sum = 0;
        // run several times
        for (int i = 0; i < iterations; i++) {
          // refresh random numbers
          for (int i = 0; i <= loop_count; i++) {
            buffer[i] = rand();
          }

          uint64_t begin = perf_read_cond_branch_misses();
          entry(loop_count, buffer);
          uint64_t elapsed = perf_read_cond_branch_misses() - begin;

          // skip warmup
          if (i >= 10) {
            double time = (double)elapsed / loop_count;
            history.push_back(time);
            sum += time;
          }
        }

        if (jit_main) {
          delete jit_main;
        }
        for (int i = 0; i < branches; i++) {
          if (jit_pages[i]) {
            delete jit_pages[i];
          }
        }

        double min = history[0];
        double max = history[0];
        for (size_t i = 0; i < history.size(); i++) {
          if (min > history[i]) {
            min = history[i];
          }
          if (max < history[i]) {
            max = history[i];
          }
        }
        fprintf(fp, "%d,%d,%d,%.2lf,%.2lf,%.2lf\n", additional_phr_bit,
                branches, dummy_branches, min, sum / history.size(), max);
        fflush(fp);
      }
    }
  }

  printf("Results are written to pht_index_bits_xor_phr.csv\n");
  delete[] buffer;
  return 0;
}
