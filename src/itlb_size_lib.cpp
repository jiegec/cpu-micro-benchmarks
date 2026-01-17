#include "include/jit.h"
#include "include/utils.h"
#include <assert.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

// args: loop count
typedef void (*gadget)(size_t);

bool avoid_hugepage_merging = false;
int stride = 64;
int fake_page_size = -1;
bool enable_itlb_misses = false;

void itlb_size(FILE *fp) {
  int loop_count = 1000;
  uint64_t min_size = 1;
  uint64_t max_size = 4096;
  bind_to_core();
  setup_perf_cycles();
  if (enable_itlb_misses) {
    setup_perf_l1itlb_misses();
    setup_perf_l2itlb_misses();
  }

  size_t page_size = getpagesize();
  if (fake_page_size != -1) {
    page_size = fake_page_size;
  }
  printf("Page Size: %ld\n", page_size);

  if (avoid_hugepage_merging) {
    printf("Avoiding hugepage merging\n");
    printf("Please disable THP via: echo never "
           ">/sys/kernel/mm/transparent_hugepage/enabled\n");
  }
  printf("Branch stride: %dB\n", stride);

  if (enable_itlb_misses) {
    fprintf(fp, "size,min,avg,max,l1itlb_misses,l2itlb_misses\n");
  } else {
    fprintf(fp, "size,min,avg,max\n");
  }
  for (uint64_t size = min_size; size <= max_size; size++) {
    gadget entry = NULL;
    jit *jit_main = NULL;
    std::vector<jit *> jit_pages;
    if (avoid_hugepage_merging) {
      // create each page separately
      // generate addresses
      std::vector<size_t> page_addrs;
      std::vector<size_t> addrs;
      size_t start_addr = 0x100000000;
      addrs.push_back(start_addr);
      page_addrs.push_back(start_addr);
      for (uint64_t i = 1; i < size; i++) {
        size_t addr = page_addrs[page_addrs.size() - 1];
        addr += page_size;
        // span over multiple cachelines to avoid hitting icache capacity
        page_addrs.push_back(addr);
        addrs.push_back(addr + (i * stride) % page_size);
      }
      uint8_t *start = (uint8_t *)start_addr;
      for (uint64_t i = 0; i < size; i++) {
        jit *jit_page = new jit((void *)page_addrs[i], page_size);

        uint8_t *begin = (uint8_t *)addrs[i];
        uint8_t *target = (uint8_t *)addrs[(i + 1) % addrs.size()];
        jit_page->set_cur(begin);
        if (i < size - 1) {
#if defined(HOST_AARCH64)
          jit_page->b(target);
#elif defined(HOST_AMD64)
          jit_page->jmp5(target);
#elif defined(HOST_PPC64LE)
          jit_page->b(target);
#endif
        } else {
#if defined(HOST_AARCH64)
          // subs x0, x0, #1
          jit_page->subs64(0, 0, 1);
          uint8_t *end = jit_page->get_cur() + 8;
          // cbnz has limited imm range
          jit_page->cbz32(0, end);
          jit_page->b(start);
          // end:
          jit_page->ret();
#elif defined(HOST_AMD64)
          jit_page->dec_r32(jit::DI);
          jit_page->jnz6(start);
          jit_page->ret();
#elif defined(HOST_PPC64LE)
          // addi 3, 3, -1
          jit_page->addi(3, 3, -1);
          // cmpdi CR0, 3, 0
          jit_page->cmpdi(0, 3, 0);
          uint8_t *end = jit_page->get_cur() + 8;
          // beq end
          jit_page->beq(end);
          // b start
          jit_page->b(start);
          // end:
          jit_main->blr();
#endif
        }
        jit_page->protect();
        jit_pages.push_back(jit_page);
      }
      entry = (gadget)start;
    } else {
      size_t mapped_size = page_size * size;
      mapped_size = (mapped_size + 0x10000) & -0x10000;
      uint8_t *start = (uint8_t *)0x7000000000;
      jit_main = new jit(start, mapped_size);
      for (uint64_t i = 0; i < size; i++) {
        // span over multiple cachelines to avoid hitting icache capacity
        uint8_t *begin = start + i * page_size + (i * stride) % page_size;
        uint8_t *target =
            start + (i + 1) * page_size + ((i + 1) * stride) % page_size;
        jit_main->set_cur(begin);
        if (i < size - 1) {
#if defined(HOST_AARCH64)
          jit_main->b(target);
#elif defined(HOST_AMD64)
          jit_main->jmp5(target);
#elif defined(HOST_PPC64LE)
          jit_main->b(target);
#endif
        } else {
#if defined(HOST_AARCH64)
          // subs x0, x0, #1
          jit_main->subs64(0, 0, 1);
          uint8_t *end = jit_main->get_cur() + 8;
          // cbnz has limited imm range
          jit_main->cbz32(0, end);
          jit_main->b(start);
          // end:
          jit_main->ret();
#elif defined(HOST_AMD64)
          jit_main->dec_r32(jit::DI);
          jit_main->jnz6(start);
          jit_main->ret();
#elif defined(HOST_PPC64LE)
          // addi 3, 3, -1
          jit_main->addi(3, 3, -1);
          // cmpdi CR0, 3, 0
          jit_main->cmpdi(0, 3, 0);
          uint8_t *end = jit_main->get_cur() + 8;
          // beq end
          jit_main->beq(end);
          // b start
          jit_main->b(start);
          // end:
          jit_main->blr();
#endif
        }
      }
      jit_main->protect();
      jit_main->dump();
      entry = (gadget)start;
    }

    std::vector<double> history;
    int iterations = 30;
    history.reserve(iterations);

    double sum = 0;
    double sum_l1itlb_misses = 0;
    double sum_l2itlb_misses = 0;
    // run several times
    for (int i = 0; i < iterations; i++) {
      uint64_t begin = perf_read_cycles();
      uint64_t begin_l1itlb_misses = 0;
      uint64_t begin_l2itlb_misses = 0;
      if (enable_itlb_misses) {
        begin_l1itlb_misses = perf_read_l1itlb_misses();
        begin_l2itlb_misses = perf_read_l2itlb_misses();
      }
      entry(loop_count);
      uint64_t elapsed = perf_read_cycles() - begin;
      uint64_t elapsed_l1itlb_misses = 0;
      uint64_t elapsed_l2itlb_misses = 0;
      if (enable_itlb_misses) {
        elapsed_l1itlb_misses = perf_read_l1itlb_misses() - begin_l1itlb_misses;
        elapsed_l2itlb_misses = perf_read_l2itlb_misses() - begin_l2itlb_misses;
      }

      // skip warmup
      if (i >= 10) {
        double time = (double)elapsed / loop_count / size;
        history.push_back(time);
        sum += time;

        if (enable_itlb_misses) {
          sum_l1itlb_misses +=
              (double)elapsed_l1itlb_misses / loop_count / size;
          sum_l2itlb_misses +=
              (double)elapsed_l2itlb_misses / loop_count / size;
        }
      }
    }

    if (jit_main) {
      delete jit_main;
    }
    for (jit *page : jit_pages) {
      delete page;
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
    if (enable_itlb_misses) {
      fprintf(fp, "%ld,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf\n", size, min,
              sum / history.size(), max, sum_l1itlb_misses / history.size(),
              sum_l2itlb_misses / history.size());
    } else {
      fprintf(fp, "%ld,%.2lf,%.2lf,%.2lf\n", size, min, sum / history.size(),
              max);
    }
    fflush(fp);
  }
}
