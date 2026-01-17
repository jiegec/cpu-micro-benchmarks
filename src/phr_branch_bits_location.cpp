#include "include/jit.h"
#include "include/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);

int main(int argc, char *argv[]) {
  int loop_count = 1000;
#if defined(HOST_AMD64)
  int min_branch_toggle = 1;
  int max_branch_toggle = 18;
  int min_dummy_branches = PHR_BRANCHES - 20;
  int max_dummy_branches = PHR_BRANCHES + 10;
#else
  int min_branch_toggle = 2;
#if defined(APPLE_SILICON)
  int max_branch_toggle = 10;
#else
  int max_branch_toggle = 30;
#endif
  int min_dummy_branches = 1;
  int max_dummy_branches = PHRB_BRANCHES + 100;
#endif

  bind_to_core();
#ifdef NO_COND_BRANCH_MISSES
  setup_perf_branch_misses();
#else
  setup_perf_cond_branch_misses();
#endif

  FILE *fp = fopen("phr_branch_bits_location.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "toggle,dummy,min,avg,max\n");
  int repeat = 2; // two branches
  for (int branch_toggle = min_branch_toggle;
       branch_toggle <= max_branch_toggle; branch_toggle++) {
    for (int dummy_branches = min_dummy_branches;
         dummy_branches <= max_dummy_branches; dummy_branches++) {
      std::vector<double> history;
      int iterations = 100;
      history.reserve(iterations);

      gadget entry = NULL;
      jit *jit_main = NULL;
#ifdef HOST_AMD64
      jit_main = new jit((void *)0x10000000L, 1024 * 1024);
      entry = (gadget)jit_main->get_cur();
      // push rdx
      jit_main->push(jit::DX);
      // push rcx
      jit_main->push(jit::CX);
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

      // two branches with opposite direction
      // only one bit differs in branch address
      // same target address
      jit_main->test32(jit::BX, jit::BX);
      jit_main->align(1 << (branch_toggle + 1), 0);
      uint8_t *first_target = jit_main->get_cur() + (1 << (branch_toggle + 1));
      if (branch_toggle >= 3) {
        // 6 bytes jnz
        jit_main->jnz6(first_target);
        for (int i = 0; i < (1 << branch_toggle) - 6; i++) {
          jit_main->nop1();
        }

        // 6 bytes jnz
        jit_main->jz6(first_target);
      } else {
        // 2 bytes jnz
        jit_main->jnz2(first_target);
        for (int i = 0; i < (1 << branch_toggle) - 2; i++) {
          jit_main->nop1();
        }

        // 2 bytes jnz
        jit_main->jz2(first_target);
      }
      jit_main->set_cur(first_target);

      // add dummy branches
      for (int i = 0; i < dummy_branches; i++) {
        uint8_t *target = jit_main->get_cur() + 64;
        jit_main->jmp2(target);
        jit_main->set_cur(target);
      }

      // add some space to avoid btb entry splitting
      for (int i = 0; i < 512; i++) {
        jit_main->nop1();
      }

      // random branch
      jit_main->test32(jit::BX, jit::BX);
      jit_main->jz2(jit_main->get_cur() + 2);

      // dec rdi
      jit_main->dec_r64(jit::DI);
      // jnz loop_begin
      jit_main->jnz6(loop_begin);

      // pop rax
      jit_main->pop(jit::AX);
      // pop rbx
      jit_main->pop(jit::BX);
      // pop rcx
      jit_main->pop(jit::CX);
      // pop rdx
      jit_main->pop(jit::DX);

      jit_main->ret();

      jit_main->protect();
#endif
#ifdef HOST_AARCH64
      // 0x7000000000: magic number on macOS, find via find_mmap_page:
      // Good region: [0x300000000, 0xf00000000]
      // Good region: [0x7000000000, 0x7fff00000000]
      jit_main = new jit((void *)0x7000000000L, 1024 * 1024 * 16);
      entry = (gadget)jit_main->get_cur();

      // loop_begin:
      uint8_t *loop_begin = jit_main->get_cur();
      // read random value
      // ldr w11, [x1, w0, uxtw #2]
      jit_main->ldr32shift2(11, 1, 0);

#ifdef NO_COND_BRANCH_MISSES
      // loop to shift phr
      // we don't have cond branch misses counter, we have to avoid
      // mispredictions
      for (int i = 0; i < 300; i++) {
        uint8_t *target = jit_main->get_cur() + 64;
        jit_main->b(target);
        jit_main->set_cur(target);
      }
#else
      // loop to shift phr
      // mov x12, 300
      jit_main->mov64(12, 300);
      // dummy_target:
      uint8_t *dummy_target = jit_main->get_cur();
      uint8_t *dummy_end = dummy_target + 32;
      // x13 = dummy_target
      jit_main->li64(13, (uint64_t)dummy_target);
      // x14 = dummy_end
      jit_main->li64(14, (uint64_t)dummy_end);
      // subs x12, x12, 1
      jit_main->subs64(12, 12, 1);
      // csel x13, x13, x14, ne
      jit_main->csel64(13, 13, 14, jit::COND_NE);
      // br x13
      jit_main->br64(13);
      // dummy_end:
      jit_main->set_cur(dummy_end);
#endif

      // two branches with opposite direction
      // only one bit differs in branch address
      // same target address
      jit_main->align(1 << (branch_toggle + 1), 0);
      uint8_t *first_target = jit_main->get_cur() + (1 << (branch_toggle + 1));
      jit_main->cbz32(11, first_target);
      for (int i = 0; i < (((1 << branch_toggle) - 4) / 4); i++) {
        jit_main->nop();
      }
      jit_main->cbnz64(11, first_target);
      jit_main->set_cur(first_target);

      // add dummy branches
      for (int i = 0; i < dummy_branches; i++) {
        uint8_t *target = jit_main->get_cur() + 64;
        jit_main->b(target);
        jit_main->set_cur(target);
      }

      // random branch
      jit_main->cbz32(11, jit_main->get_cur() + 4);

      // x12 = loop_begin
      jit_main->li64(12, (uint64_t)loop_begin);
      // x13 = end_loop
      uint8_t *end_loop = jit_main->get_cur() + 64;
      jit_main->li64(13, (uint64_t)end_loop);
      // subs x0, x0, #1
      jit_main->subs64(0, 0, 1);
      // csel x12, x12, x13, ne
      jit_main->csel64(12, 12, 13, jit::COND_NE);
      // br x12
      jit_main->br64(12);

      // end_loop:
      jit_main->set_cur(end_loop);
      jit_main->ret();

      jit_main->protect();
#endif

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
          double time = (double)elapsed / loop_count / repeat;
          history.push_back(time);
          sum += time;
        }
      }

      if (jit_main) {
        delete jit_main;
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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", branch_toggle, dummy_branches,
              min, sum / history.size(), max);
      fflush(fp);
    }
  }

  printf("Results are written to phr_branch_bits_location.csv\n");
  delete[] buffer;
  return 0;
}
