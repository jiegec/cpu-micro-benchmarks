#include "include/utils.h"
#include <assert.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <tuple>
#include <unistd.h>
#include <vector>

// use with generate_gadget tool

// defined in gen_ghr2_test()
// args: loop count, buffer
typedef void (*gadget)(size_t, uint32_t *);
extern "C" {
extern gadget ghr2_gadgets[];
}

const int min_branch_align = 5;
const int max_branch_align = 20;
const int min_target_align = 1;
const int max_target_align = 8;

std::tuple<size_t, gadget> jit(int branch_align, int target_align) {
  // jit code for ghr2 structure:
  // prolog
  // flush ghr:
  // 256 * jump to next aligned jump
  // jump to first jnz
  // jnz 2f:
  // fallthrough:
  // jnz 3f:
  // 3: dec %rdi
  // jnz 1b
  // pop rbx
  // ret
  //
  // 2:
  // jnz 3f:
  // 3: dec %rdi
  // jnz 1b
  // pop rbx
  // ret

  printf("Branch align: %d\n", branch_align);
  printf("Target align: %d\n", target_align);

  // mmap 1G for jitted code
  size_t base =
      (size_t)mmap(NULL, 1 * 1024 * 1024 * 1024, PROT_WRITE | PROT_EXEC,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  size_t mem = base;

  // align up to max_branch_align
  mem++;
  while (mem & ((1 << max_branch_align) - 1)) {
    mem++;
  }

  // retract 6+5 bytes for prolog
  mem -= 6 + 5;

  // this is also function entry
  // prolog
  unsigned char *p = (unsigned char *)mem;
  // 0x53: push %rbx
  *p++ = 0x53;
  unsigned char *loop_entry = p;
  // 0x8b 0x1c 0xbe: mov (%rsi, %rdi, 4), %ebx
  *p++ = 0x8b;
  *p++ = 0x1c;
  *p++ = 0xbe;
  // 0x85 0xdb: test %ebx, %ebx
  *p++ = 0x85;
  *p++ = 0xdb;

  // 256 always taken branches
  // the end of the branches are aligned to 2**max_branch_align
  int taken_branches = 256;
  printf("Dummy branch addr: %p\n", p + 5);
  for (int i = 0; i < taken_branches; i++) {
    // 0xe9 OFF OFF OFF OFF: jmp 2f
    size_t offset = (1 << max_branch_align) - 5;
    *p++ = 0xe9;
    *p++ = offset & 0xFF;
    *p++ = (offset >> 8) & 0xFF;
    *p++ = (offset >> 16) & 0xFF;
    *p++ = (offset >> 24) & 0xFF;

    // fill invalid instructions
    for (int j = 0; j < (1 << max_branch_align) - 5; j++) {
      *p++ = 0xf4;
    }
  }

  // first branch
  // add some 1s to higher bits

  size_t jnz_offset; // end addr of jnz
  size_t target_offset;

  // nop * (jnz_offset - 6)
  // jnz 2f:
  // align to max_branch_align
  // nop * target_offset
  // 2:

  if (branch_align < max_branch_align) {
    jnz_offset = (1 << (max_branch_align - branch_align)) - 1;
    jnz_offset <<= branch_align;
  } else {
    jnz_offset = 1 << max_branch_align;
  }
  // do not under/overflow
  assert(jnz_offset > 6);

  if (target_align < max_branch_align) {
    target_offset = (1 << (max_branch_align - target_align)) - 1;
    target_offset <<= target_align;
  } else {
    target_offset = 0;
  }
  target_offset += 1 << max_branch_align;

  // jump to first jnz
  // 0xe9 OFF OFF OFF OFF: jmp 2f
  size_t offset = jnz_offset - 6;
  *p++ = 0xe9;
  *p++ = offset & 0xFF;
  *p++ = (offset >> 8) & 0xFF;
  *p++ = (offset >> 16) & 0xFF;
  *p++ = (offset >> 24) & 0xFF;

  // fill 0xf4 until jnz_offset - 6
  for (size_t i = 0; i < jnz_offset - 6; i++) {
    *p++ = 0xf4;
  }

  // 0x0f 0x85 OFF OFF OFF OFF: jnz 2f
  printf("Branch addr: %p\n", p + 6);
  offset = target_offset - jnz_offset;
  *p++ = 0x0f;
  *p++ = 0x85;
  *p++ = offset & 0xFF;
  *p++ = (offset >> 8) & 0xFF;
  *p++ = (offset >> 16) & 0xFF;
  *p++ = (offset >> 24) & 0xFF;

  // second branch in fallthrough
  // 0x0f 0x85 OFF OFF OFF OFF: jnz 2f
  offset = 0;
  *p++ = 0x0f;
  *p++ = 0x85;
  *p++ = offset & 0xFF;
  *p++ = (offset >> 8) & 0xFF;
  *p++ = (offset >> 16) & 0xFF;
  *p++ = (offset >> 24) & 0xFF;

  // epilogue
  // 0x48 0xff 0xcf: dec %rdi
  *p++ = 0x48;
  *p++ = 0xff;
  *p++ = 0xcf;

  // 0x0f 0x85 OFF OFF OFF OFF: jnz 2f
  offset = (size_t)loop_entry - (size_t)p - 6;
  *p++ = 0x0f;
  *p++ = 0x85;
  *p++ = offset & 0xFF;
  *p++ = (offset >> 8) & 0xFF;
  *p++ = (offset >> 16) & 0xFF;
  *p++ = (offset >> 24) & 0xFF;

  // 0x5b: pop %rbx
  *p++ = 0x5b;

  // 0xc3: ret
  *p++ = 0xc3;

  // fill 0xf4 to target
  for (size_t i = jnz_offset + 6 + 3 + 6 + 1 + 1; i < target_offset; i++) {
    *p++ = 0xf4;
  }
  printf("Target addr: %p\n", p);

  // second branch in taken direction
  // 0x0f 0x85 OFF OFF OFF OFF: jnz 2f
  offset = 0;
  *p++ = 0x0f;
  *p++ = 0x85;
  *p++ = offset & 0xFF;
  *p++ = (offset >> 8) & 0xFF;
  *p++ = (offset >> 16) & 0xFF;
  *p++ = (offset >> 24) & 0xFF;

  // epilogue
  // 0x48 0xff 0xcf: dec %rdi
  *p++ = 0x48;
  *p++ = 0xff;
  *p++ = 0xcf;

  // 0x0f 0x85 OFF OFF OFF OFF: jnz 2f
  offset = (size_t)loop_entry - (size_t)p - 6;
  *p++ = 0x0f;
  *p++ = 0x85;
  *p++ = offset & 0xFF;
  *p++ = (offset >> 8) & 0xFF;
  *p++ = (offset >> 16) & 0xFF;
  *p++ = (offset >> 24) & 0xFF;

  // 0x5b: pop %rbx
  *p++ = 0x5b;

  // 0xc3: ret
  *p++ = 0xc3;

  return {base, (gadget)mem};
}

int main(int argc, char *argv[]) {
  int loop_count = 20000;

  nice(-20);
  bind_to_core();
  setup_perf_branch_misses();
  FILE *fp = fopen("ghr2_size.csv", "w");
  assert(fp);

  uint32_t *buffer = new uint32_t[loop_count + 1];

  fprintf(fp, "branch,target,min,avg,max\n");
  int repeat = 2; // two branches
  for (int branch_align = min_branch_align; branch_align <= max_branch_align;
       branch_align++) {
    for (int target_align = min_target_align; target_align <= max_target_align;
         target_align++) {
      for (int i = 0; i <= loop_count; i++) {
        buffer[i] = rand() % 2;
      }

      std::vector<double> history;
      int iterations = 20;
      history.reserve(iterations);

      size_t mem;
      gadget g;
      std::tie(mem, g) = jit(branch_align, target_align);

      double sum = 0;
      // run several times
      for (int i = 0; i < iterations; i++) {
        uint64_t begin = perf_read_branch_misses();
        g(loop_count, buffer);
        uint64_t elapsed = perf_read_branch_misses() - begin;

        // skip warmup
        if (i >= 10) {
          double time = (double)elapsed / loop_count / repeat;
          history.push_back(time);
          sum += time;
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
      fprintf(fp, "%d,%d,%.2lf,%.2lf,%.2lf\n", branch_align, target_align, min,
              sum / history.size(), max);
      fflush(fp);

      munmap((void *)mem, 1 * 1024 * 1024 * 1024);
    }
  }

  printf("Results are written to ghr2_size.csv\n");
  delete[] buffer;
  return 0;
}
