#include "include/jit.h"
#include "include/utils.h"
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

// use with generate_gadget tool

// defined in gen_pht_index_bits_xor_test()
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget pht_index_bits_xor_gadgets[];
}

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

  // order by phr_bit
  bool operator<(const struct phr_injection &other) const {
    return phr_bit < other.phr_bit;
  }
};

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  // match gen_pht_index_bits_xor_test
  // test PC[9] vs PHR bits
  int min_branches = 1;
  int max_branches = 9;
  int min_dummy_branches = 0;
  int max_dummy_branches = PHR_BRANCHES;

  // which table to test?
  int test_table = 1;
  // test PHRT or PHRB
  bool inject_target = true;
  int opt;
  int index_bit_override = -1;
  while ((opt = getopt(argc, argv, "t:i:b")) != -1) {
    switch (opt) {
    case 't':
      sscanf(optarg, "%d", &test_table);
      break;
    case 'i':
      sscanf(optarg, "%d", &index_bit_override);
      break;
    case 'b':
      inject_target = false;
      break;
    default:
      fprintf(stderr, "Usage: %s [-t table] [-i index_bit]\n", argv[0]);
      fprintf(stderr,
              "\t-t table: which table to test? 1 for longest, 2 for 2nd\n");
      fprintf(stderr, "\t-i index_bit: toggle which index bit in pc?\n");
      fprintf(stderr, "\t-b: inject PHRB instead of PHRT\n");
      exit(EXIT_FAILURE);
    }
  }
  printf("Test table #%d\n", test_table);

  int index_bit;
#ifdef QUALCOMM_ORYON
  // Oryon: PC[7/9] xor with PHR
  if (test_table == 1) {
    index_bit = 7;
  } else {
    index_bit = 9;
  }
#else
  // Firestorm: PC[9] xor with PHR
  index_bit = 9;
#endif
  if (index_bit_override != -1) {
    index_bit = index_bit_override;
  }
  printf("Testing PC[%d] xor with PHR\n", index_bit);
  printf("Inject %s\n", inject_target ? "PHRT" : "PHRB");

  bind_to_core();
  setup_perf_cond_branch_misses();
  FILE *fp = fopen("pht_index_bits_xor.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "branches,dummy,min,avg,max\n");
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
      uintptr_t target_branches[64];
      jit_main = new jit(16 * 1024);
      entry = (gadget)jit_main->get_cur();

      // leave space for registers
      // sub sp, sp, #0x40
      jit_main->sub64(31, 31, 0x40);
      // stp x20, x21, [sp, #0x20]
      jit_main->stp64(20, 21, 31, 0x20);

      // loop_begin:
      uint8_t *loop_begin = jit_main->get_cur();

      // read random value to w11
      // ldr w11, [x1, w0, uxtw #2]
      jit_main->ldr32shift2(11, 1, 0);

      // shift phr
      for (int i = 0; i < 200; i++) {
        jit_main->b(jit_main->get_cur() + 4);
      }

      int phr_index_bit = -1;
      int history_bits = -1;
      int assoc = 4;
#ifdef QUALCOMM_ORYON
      // test longest tage table
      if (test_table == 1) {
        phr_index_bit = 95;
      } else if (test_table == 2) {
        // test second longest tage table
        history_bits = 52;
        phr_index_bit = 51;
      } else if (test_table == 3) {
        // test third longest tage table
        history_bits = 27;
        phr_index_bit = 26;
      } else if (test_table == 4) {
        // test fourth longest tage table
        history_bits = 14;
        phr_index_bit = 13;
      } else if (test_table == 5) {
        // test fifth longest tage table
        history_bits = 7;
        phr_index_bit = 6;
        assoc = 6;
      }
#else
      // test longest tage table
      if (test_table == 1) {
        phr_index_bit = 99;
      } else if (test_table == 2) {
        // test second longest tage table
        history_bits = 57;
        phr_index_bit = 56;
      } else if (test_table == 3) {
        // test third longest tage table
        history_bits = 32;
        phr_index_bit = 31;
      } else if (test_table == 4) {
        // test fourth longest tage table
        history_bits = 18;
        phr_index_bit = 17;
      } else if (test_table == 5) {
        // test fifth longest tage table
        history_bits = 11;
        phr_index_bit = 10;
        assoc = 6;
      } else if (test_table == 6) {
        // test sixth longest tage table
        history_bits = 11; // let's consider the two tables together
        // history_bits = 6;
        phr_index_bit = 3;
        assoc = 6;
        // speedup and show more detail with bigger assoc
        max_branches = 17;
        max_dummy_branches = 16;
      }
#endif

      std::vector<phr_injection> injections;

      // fill random bits in longer tables
      if (history_bits != -1) {
        for (int i = 0; i < 15; i++) {
          injections.push_back(phr_injection{
              .phr_bit = history_bits + i,
              .k_bit = 7 + i,
          });
        }
      }

      // to test lower phr bits,
      // we inject a random bit at PHR[phr_index_bit]
      injections.push_back(phr_injection{
          .phr_bit = phr_index_bit,
          .k_bit = 1,
      });

      // handle injections above
      std::sort(injections.begin(), injections.end());
      for (int i = injections.size() - 1; i >= 0; i--) {
        inject_w11_phrt(jit_main, injections[i].k_bit);
        int shift;
        if (i > 0) {
          shift = injections[i].phr_bit - injections[i - 1].phr_bit - 1;
        } else {
          shift =
              injections[i].phr_bit - dummy_branches - (inject_target ? 1 : 3);
        }
        for (int j = 0; j < shift; j++) {
          jit_main->b(jit_main->get_cur() + 4);
        }
      }

      // jump to branches randomly
      // x13 = x11 / 4, the lower two bits are used to inject k
      jit_main->lsr64(13, 11, 2);
      // x15 = branches
      // x14 = x13 / x15
      jit_main->mov64(15, branches);
      jit_main->udiv64(14, 13, 15);
      // x20 = x13 % x14
      jit_main->msub64(20, 14, 15, 13);
      // x16 = target_branches
      jit_main->li64(16, (uint64_t)&target_branches[0]);
      // x20 = x16[x20]
      jit_main->ldr64shift3(20, 16, 20);
      // x20 is the address of target branch

      // set x10 = x11[0] xor x11[1]
      // x10 = x11 & 1
      jit_main->mov64(10, 1);
      jit_main->and64(10, 11, 10);
      // x13 = x11 & 2
      jit_main->mov64(13, 2);
      jit_main->and64(13, 11, 13);
      // x13 = x13 / 2
      jit_main->lsr64(13, 13, 1);
      // x10 = x10 ^ x13
      jit_main->eor64(10, 10, 13);

      if (inject_target) {
        if (dummy_branches > 0) {
          // inject w11[0] to phrt LSB
          inject_w11_phrt(jit_main, 0);

          // shift phr by dummy_branches-1 time
          // there is one branch later
          for (int i = 0; i < dummy_branches - 1; i++) {
            jit_main->b(jit_main->get_cur() + 4);
          }
          // br x20
          jit_main->br64(20);
        } else {
          // we inject PHRT[0] = w11[0] here
          // x12 = x11 & (1 << 0)
          jit_main->mov64(12, (1 << 0));
          jit_main->and64(12, 11, 12);
          // x12 = x12 << 2
          jit_main->lsl64(12, 12, 2);
          // x20 = x20 + x12
          jit_main->addr64(20, 20, 12);
          // br x20
          jit_main->br64(20);
        }
      } else {
        if (dummy_branches > 0) {
          // inject w11[0] to phrb LSB
          inject_w11_phrb(jit_main, 0, false);
          // shift phr by dummy_branches-1 time
          // there is one branch later
          for (int i = 0; i < dummy_branches - 1; i++) {
            jit_main->b(jit_main->get_cur() + 4);
          }
          // br x20
          jit_main->br64(20);
        } else {
          // inject w11[0] to phrb LSB and jump to x20
          inject_w11_phrb(jit_main, 0, true);
        }
      }

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

      // ldp x20, x21, [sp, #0x20]
      jit_main->ldp64(20, 21, 31, 0x20);
      // add sp, sp, #0x40
      jit_main->add64(31, 31, 0x40);
      jit_main->ret();
      // jit_main->dump();

      // following Half & Half: use lower bits to differentiate
      // it does work by avoiding using pc bits in index
      int offsets[64];
      // when assoc = 4, first 4 and second 4 are different in PC[9]
      for (int i = 0; i < assoc; i++) {
        offsets[i] = 4 * i;
      }
      for (int i = assoc; i < 64; i++) {
        offsets[i] = 4 * i + (1 << index_bit);
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
        // add extra nop for injecting PHRT[0]
        jit_pages[i]->nop();

        // add conditional branch on w10
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
      for (int i = 0; i < 16; i++) {
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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", branches, dummy_branches, min,
              sum / history.size(), max);
      fflush(fp);
    }
  }

  printf("Results are written to pht_index_bits_xor.csv\n");
  delete[] buffer;
  return 0;
}
