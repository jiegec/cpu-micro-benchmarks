#include "include/jit.h"
#include "include/utils.h"
#include <assert.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

// defined in gen_phr_target_bits_location_test()
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget phr_target_bits_location_gadgets[];
}

int main(int argc, char *argv[]) {
  int loop_count = 1000;
  // match gen_phr_target_bits_location_test
#if defined(HOST_AMD64)
  bool use_jit = true;
  int min_target_toggle = 0;
  int max_target_toggle = 18;
  int min_dummy_branches = PHR_BRANCHES - 20;
  int max_dummy_branches = PHR_BRANCHES + 10;
#else
  bool use_jit = false;
  int min_target_toggle = 2;
  // at most 48 virtual address bits
#ifdef __APPLE__
  // cannot surpass page size using ld
  int max_target_toggle = 13;
#else
  int max_target_toggle = 14;
#endif
  int min_dummy_branches = PHR_BRANCHES - 35;
  int max_dummy_branches = PHR_BRANCHES + 0;
#endif

  int opt;
  while ((opt = getopt(argc, argv, "j")) != -1) {
    switch (opt) {
    case 'j':
      use_jit = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-j]\n", argv[0]);
      fprintf(stderr, "\t-j: use jit compilation\n");
      exit(EXIT_FAILURE);
    }
  }
  printf("Using %s\n", use_jit ? "JIT" : "pregenerated assembly");
#ifdef HOST_AARCH64
  if (use_jit) {
    max_target_toggle = 32;
  }
#endif

  bind_to_core();
#ifdef NO_COND_BRANCH_MISSES
  setup_perf_branch_misses();
#else
  setup_perf_cond_branch_misses();
#endif

  FILE *fp = fopen("phr_target_bits_location.csv", "w");
  assert(fp);

  printf("Using %s\n", use_jit ? "JIT mode" : "pregenerated assembly");
  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "toggle,dummy,min,avg,max\n");
  int gadget_index = 0;
  for (int target_toggle = min_target_toggle;
       target_toggle <= max_target_toggle; target_toggle++) {
    for (int dummy_branches = min_dummy_branches;
         dummy_branches <= max_dummy_branches; dummy_branches++) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      // main part
      gadget entry = NULL;
      jit *jit_main = NULL;
      jit *jit_first_br = NULL;
      jit *jit_second_br = NULL;
      if (use_jit) {
#ifdef HOST_AMD64
        jit_main = new jit((void *)0x10000000L, 1024 * 1024);
        entry = (gadget)jit_main->get_cur();
        // push rbx
        jit_main->push(jit::BX);
        // push rax
        jit_main->push(jit::AX);

        // loop_begin:
        uint8_t *loop_begin = jit_main->get_cur();
        // read random value
        // mov ebx, [rsi+rdi*4]
        jit_main->load32_shift2(jit::BX, jit::SI, jit::DI);

        // loop to shift phr
        // mov eax, 200
        jit_main->movi32(jit::AX, 200);
        // dummy_target:
        uint8_t *dummy_target = jit_main->get_cur();
        uint8_t *dummy_end = dummy_target + 32;
        // lea rcx, dummy_target
        jit_main->movabs8(jit::CX, (uint64_t)dummy_target);
        // lea rdx, dummy_end
        jit_main->movabs8(jit::DX, (uint64_t)dummy_end);
        // dec eax
        jit_main->dec_r32(jit::AX);
        // cmovz rcx, rdx
        jit_main->cmovz64(jit::CX, jit::DX);
        // jmp rcx
        jit_main->jmpr64(jit::CX);
        jit_main->set_cur(dummy_end);

        // inject ebx to target address
        // differ in one bit
        jit_first_br = new jit((void *)0x20000000L, 1024 * 1024);
        uint8_t *first_target_1 = jit_first_br->get_cur();
        uint8_t *first_target_2 = first_target_1 + (1 << target_toggle);
        assert(((uint64_t)first_target_1 ^ (uint64_t)first_target_2) ==
               (uint64_t)(1 << target_toggle));

        // lea rax, first_target_1
        jit_main->movabs8(jit::AX, (uint64_t)first_target_1);
        // lea rcx, first_target_2
        jit_main->movabs8(jit::CX, (uint64_t)first_target_2);
        // test ebx, ebx
        jit_main->test32(jit::BX, jit::BX);
        // cmovz rax, rcx
        jit_main->cmovz64(jit::AX, jit::CX);
        // jmp rax
        jit_main->jmpr64(jit::AX);

        jit_first_br->set_cur(first_target_1);
        while (jit_first_br->get_cur() < first_target_2) {
          jit_first_br->nop1();
        }

        // add dummy branches
        for (int i = 0; i < dummy_branches; i++) {
          uint8_t *target = jit_first_br->get_cur() + 64;
          jit_first_br->jmp2(target);
          jit_first_br->set_cur(target);
        }

        // add some space to avoid btb entry splitting
        for (int i = 0; i < 512; i++) {
          jit_first_br->nop1();
        }

        // random branch
        jit_first_br->test32(jit::BX, jit::BX);
        jit_first_br->jz2(jit_first_br->get_cur() + 2);

        // dec rdi
        jit_first_br->dec_r64(jit::DI);
        // jnz loop_begin
        jit_first_br->jnz6(loop_begin);

        // pop rax
        jit_first_br->pop(jit::AX);
        // pop rbx
        jit_first_br->pop(jit::BX);

        jit_first_br->ret();

        jit_first_br->protect();
#endif
#ifdef HOST_AARCH64
        jit_main = new jit(1024 * 1024);
        entry = (gadget)jit_main->get_cur();

        // loop_begin:
        uint8_t *loop_begin = jit_main->get_cur();
        // ldr w11, [x1, w0, uxtw #2]
        jit_main->ldr32shift2(11, 1, 0);

#ifdef NO_COND_BRANCH_MISSES
        // loop to shift phr
        // we don't have cond branch misses counter, we have to avoid
        // mispredictions
        for (int i = 0; i < 100; i++) {
          uint8_t *target = jit_main->get_cur() + 64;
          jit_main->b(target);
          jit_main->set_cur(target);
        }
#else
        // shift phr
        // mov x12, 200
        jit_main->mov64(12, 200);
        // dummy_target:
        uint8_t *dummy_target = jit_main->get_cur();
        // li x13, dummy_target
        jit_main->li64(13, (uintptr_t)dummy_target);
        // dummy_end:
        uint8_t *dummy_end = dummy_target + 128;
        // li x14, dummy_end
        jit_main->li64(14, (uintptr_t)dummy_end);
        // subs x12, x12, 1
        jit_main->subs64(12, 12, 1);
        // csel x13, x13, x14, ne
        jit_main->csel64(13, 13, 14, jit::COND_NE);
        // br x13
        jit_main->br64(13);
        // fast forward to dummy_end
        jit_main->set_cur(dummy_end);
#endif

        uintptr_t first_target;
        uintptr_t second_target;
// separate assumes B[14+] not input
#if defined(APPLE_M1_FIRESTORM) || defined(APPLE_M2_AVALANCHE) ||              \
    defined(QUALCOMM_ORYON)
        bool separate = target_toggle > 14; // must larger than page size
#else
        bool separate = false;
#endif
        if (separate) {
          // use 0 or 1<<(target_toggle+1) for first_target
          first_target = (uintptr_t)1 << (target_toggle + 1);
          second_target = first_target + ((uintptr_t)1 << target_toggle);
        } else {
          first_target = (uintptr_t)jit_main->get_cur();
          first_target += 1024;
          uintptr_t align = 1 << (target_toggle + 1);
          first_target = (first_target + align - 1) & -align;
          second_target = first_target + (1 << target_toggle);
        }
        // li x12, first_target
        jit_main->li64(12, (uintptr_t)first_target);
        // li x13, second_target
        jit_main->li64(13, (uintptr_t)second_target);
        // cmp w11, 0
        jit_main->cmp32(11, 0);
        // csel x12, x12, x13, eq
        jit_main->csel64(12, 12, 13, jit::COND_EQ);
        // br x12
        jit_main->br64(12);

        // two targets
        if (separate) {
          jit_first_br = new jit((void *)first_target, 1024 * 16);
          jit_second_br = new jit((void *)second_target, 1024 * 16);
          uint8_t *target_end = jit_main->get_cur();
          // jump to target_end in two brs
          // li x13, target_end
          jit_first_br->li64(13, (uint64_t)target_end);
          // br x13
          jit_first_br->br64(13);
          jit_first_br->dump("jit_first_br.bin");
          jit_first_br->protect();

          // li x13, target_end
          jit_second_br->li64(13, (uint64_t)target_end);
          // br x13
          jit_second_br->br64(13);
          jit_second_br->dump("jit_second_br.bin");
          jit_second_br->protect();
        } else {
          jit_main->set_cur((uint8_t *)first_target);
          for (uintptr_t i = 0; i < (second_target - first_target) / 4; i++) {
            jit_main->nop();
          }
          jit_main->set_cur((uint8_t *)second_target);
        }

        // if separate, one extra branch
        for (int i = 0; i < dummy_branches - separate; i++) {
          uint8_t *cur = jit_main->get_cur();
          uint8_t *next = cur + 64;
          jit_main->b(next);
          jit_main->set_cur(next);
        }
        // cbz w11, third_target
        uint8_t *third_target = jit_main->get_cur() + 4;
        jit_main->cbz32(11, third_target);

        // jump to loop begin or loop end
        // li x12, loop_begin
        jit_main->li64(12, (uint64_t)loop_begin);
        // li x13, end_loop
        uint8_t *end_loop = jit_main->get_cur() + 128;
        jit_main->li64(13, (uint64_t)end_loop);
        // subs x0, x0, #1
        jit_main->subs64(0, 0, 1);
        // csel x12, x12, x13, ne
        jit_main->csel64(12, 12, 13, jit::COND_NE);
        // br x12
        jit_main->br64(12);
        // end_loop:
        jit_main->set_cur(end_loop);

        // ret
        jit_main->ret();
#endif
        jit_main->dump("jit_main.bin");
        jit_main->protect();
      } else {
        entry = phr_target_bits_location_gadgets[gadget_index];
      }

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        for (int i = 0; i <= loop_count; i++) {
          buffer[i] = rand() % 2;
        }

#ifdef NO_COND_BRANCH_MISSES
        uint64_t begin = perf_read_branch_misses();
#else
        uint64_t begin = perf_read_cond_branch_misses();
#endif
        entry(loop_count, buffer);
#ifdef NO_COND_BRANCH_MISSES
        uint64_t elapsed = perf_read_branch_misses() - begin;
#else
        uint64_t elapsed = perf_read_cond_branch_misses() - begin;
#endif

        // skip warmup
        if (i >= 10) {
          double time = (double)elapsed / loop_count;
          history.push_back(time);
          sum += time;
        }
      }
      gadget_index++;

      if (jit_main) {
        delete jit_main;
      }
      if (jit_first_br) {
        delete jit_first_br;
      }
      if (jit_second_br) {
        delete jit_second_br;
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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", target_toggle, dummy_branches,
              min, sum / history.size(), max);
      fflush(fp);
    }
  }

  printf("Results are written to phr_target_bits_location.csv\n");
  delete[] buffer;
  return 0;
}
