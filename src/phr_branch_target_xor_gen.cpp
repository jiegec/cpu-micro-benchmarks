#include "include/utils.h"

// https://cseweb.ucsd.edu/~dstefan/pubs/yavarzadeh:2023:half.pdf
int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);

#if defined(HOST_AMD64)
  int min_branch_toggle = 1;
  int max_branch_toggle = 15;
  int min_target_toggle = 0;
  int max_target_toggle = 12;
#else
  int min_branch_toggle = 2;
  int max_branch_toggle = 18;
  int min_target_toggle = 2;
  int max_target_toggle = 18;
#endif

  // args: loop count, random array
#if defined(HOST_AMD64)
  nasm = true;
  fprintf(fp, "section .text\n");
#else
  fprintf(fp, ".text\n");
#endif
  for (int branch_toggle = min_branch_toggle;
       branch_toggle <= max_branch_toggle; branch_toggle++) {
    for (int target_toggle = min_target_toggle;
         target_toggle <= max_target_toggle; target_toggle++) {
#if defined(HOST_AMD64)
      fprintf(fp, "global phr_branch_target_xor_%d_%d\n", branch_toggle,
              target_toggle);
      fprintf(fp, "align 32\n");
      fprintf(fp, "phr_branch_target_xor_%d_%d:\n", branch_toggle,
              target_toggle);

      // save registers
      fprintf(fp, "\tpush rbx\n");
      fprintf(fp, "\tpush rax\n");

      fprintf(fp, "\tphr_branch_target_xor_%d_%d_loop_begin:\n", branch_toggle,
              target_toggle);

      // read random value
      fprintf(fp, "\tmov eax, [rsi+rdi*4]\n");

      // loop to shift phr
      for (int i = 0; i < 200; i++) {
        fprintf(fp, "\tjmp phr_branch_target_xor_%d_%d_dummy_target_%d\n",
                branch_toggle, target_toggle, i);
        fprintf(fp, "\talign 64\n");
        fprintf(fp, "\tphr_branch_target_xor_%d_%d_dummy_target_%d:\n",
                branch_toggle, target_toggle, i);
      }

      // two branches in the opposite direction:
      // B[branch_toggle] & T[target_toggle] differs

      fprintf(fp,
              "\tlea rcx, [rel phr_branch_target_xor_%d_%d_first_target_2]\n",
              branch_toggle, target_toggle);

      // first branch
      // place alignment on branch (end) & target
      // force B[branch_toggle]=0
      fprintf(fp, "\talign %d\n", (1 << (branch_toggle + 2)));
      // avoid negative nops
      emit_nasm_nops(fp, (1 << (branch_toggle + 2)) +
                             (1 << (branch_toggle + 1)) - 7);
      // 2 bytes test eax
      fprintf(fp, "\ttest eax, eax\n");

      // 6 bytes jnz
      // last byte of jnz aligned to 1<<(branch_toggle+1)
      fprintf(fp, "\tdb 0x0f\n");
      fprintf(fp, "\tdb 0x85\n");
      fprintf(fp, "\tdd phr_branch_target_xor_%d_%d_first_target - $ - 4\n",
              branch_toggle, target_toggle);

      emit_nasm_nops(fp, (1 << branch_toggle) - 2);

      // 2 bytes jmp rcx(points to first_target_2)
      // last byte of jmp aligned to 1<<branch_toggle
      fprintf(fp, "\tjmp rcx\n");

      fprintf(fp, "\talign %d\n", (1 << (target_toggle + 1)));
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_first_target:\n",
              branch_toggle, target_toggle);
      emit_nasm_nops(fp, (1 << target_toggle));
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_first_target_2:\n",
              branch_toggle, target_toggle);

      // loop to shift phr
      for (int i = 0; i < 10; i++) {
        fprintf(fp, "\tjmp phr_branch_target_xor_%d_%d_dummy_target_2_%d\n",
                branch_toggle, target_toggle, i);
        fprintf(fp, "\talign 64\n");
        fprintf(fp, "\tphr_branch_target_xor_%d_%d_dummy_target_2_%d:\n",
                branch_toggle, target_toggle, i);
      }

      // second random branch
      fprintf(fp, "\talign 64\n");
      emit_nasm_nops(fp, 64);
      fprintf(fp, "\tjnz phr_branch_target_xor_%d_%d_second_target\n",
              branch_toggle, target_toggle);
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_second_target:\n",
              branch_toggle, target_toggle);

      fprintf(fp, "\tdec rdi\n");
      fprintf(fp, "\tjnz phr_branch_target_xor_%d_%d_loop_begin\n",
              branch_toggle, target_toggle);

      // restore regs
      fprintf(fp, "\tpop rax\n");
      fprintf(fp, "\tpop rbx\n");
      fprintf(fp, "\tret\n");
#elif defined(HOST_AARCH64)
      fprintf(fp, ".global phr_branch_target_xor_%d_%d\n", branch_toggle,
              target_toggle);
      fprintf(fp, "phr_branch_target_xor_%d_%d:\n", branch_toggle,
              target_toggle);

      fprintf(fp, "\tphr_branch_target_xor_%d_%d_loop_begin:\n", branch_toggle,
              target_toggle);

      // read random value
      fprintf(fp, "\tldr w11, [x1, w0, uxtw #2]\n");

#ifdef NO_COND_BRANCH_MISSES
      // loop to shift phr
      // we don't have cond branch misses counter, we have to avoid
      // mispredictions
      for (int i = 0; i < 300; i++) {
        fprintf(fp, "\tb 1f\n");
        fprintf(fp, "\t.balign 64\n");
        fprintf(fp, "\t1:\n");
      }
#else
      // NOTE: generate more loops although they contribute to branch misses due
      // to history limit. we remove the extra branch misses in
      // phr_branch_target_xor.cpp. do not generate a chain of loops: btb
      // will become a bottleneck.
      fprintf(fp, "\tmov x12, 200\n");
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_dummy_target:\n",
              branch_toggle, target_toggle);
      // x13 = dummy_target
      arm64_la(fp, 13, "phr_branch_target_xor_%d_%d_dummy_target",
               branch_toggle, target_toggle);
      // x14 = dummy_end
      arm64_la(fp, 14, "phr_branch_target_xor_%d_%d_dummy_end", branch_toggle,
               target_toggle);
      fprintf(fp, "\tsubs x12, x12, 1\n");
      fprintf(fp, "\tcsel x13, x13, x14, ne\n");
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\tbr x13\n");
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_dummy_end:\n", branch_toggle,
              target_toggle);
#endif

      // two branches with opposite direction
      // only one bit differs in branch address
      // same target address
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\t.balign %d\n", (1 << (branch_toggle + 1)));
      fprintf(fp, "\tcbz w11, phr_branch_target_xor_%d_%d_first_target\n",
              branch_toggle, target_toggle);
      // toggle one bit in address
      fprintf(fp, "\t.rep %d\n", (((1 << branch_toggle) - 4) / 4));
      fprintf(fp, "\tnop\n");
      fprintf(fp, "\t.endr\n");
      fprintf(fp, "\tcbnz w11, phr_branch_target_xor_%d_%d_first_target_2\n",
              branch_toggle, target_toggle);

      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\t.balign %d\n", (1 << (target_toggle + 1)));
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_first_target:\n",
              branch_toggle, target_toggle);
      fprintf(fp, "\t.rep %d\n", (((1 << target_toggle)) / 4));
      fprintf(fp, "\tnop\n");
      fprintf(fp, "\t.endr\n");
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_first_target_2:\n",
              branch_toggle, target_toggle);

      // add dummy branches
      fprintf(fp, "\t.balign 64\n");
      for (int i = 0; i < 10; i++) {
        fprintf(fp, "\tb phr_branch_target_xor_%d_%d_dummy_target_2_%d\n",
                branch_toggle, target_toggle, i);
        fprintf(fp, "\t.balign 64\n");
        fprintf(fp, "\tphr_branch_target_xor_%d_%d_dummy_target_2_%d:\n",
                branch_toggle, target_toggle, i);
      }

      // second random branch
      fprintf(fp, "\t.balign 64\n");
      fprintf(fp, "\tcbz w11, phr_branch_target_xor_%d_%d_second_target\n",
              branch_toggle, target_toggle);
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_second_target:\n",
              branch_toggle, target_toggle);

      fprintf(fp, "\t.balign 64\n");
      // x12 = loop_begin
      arm64_la(fp, 12, "phr_branch_target_xor_%d_%d_loop_begin", branch_toggle,
               target_toggle);
      // x13 = end_loop
      arm64_la(fp, 13, "phr_branch_target_xor_%d_%d_end_loop", branch_toggle,
               target_toggle);
      fprintf(fp, "\tsubs x0, x0, #1\n");
      fprintf(fp, "\tcsel x12, x12, x13, ne\n");
      fprintf(fp, "\tbr x12\n");
      fprintf(fp, "\tphr_branch_target_xor_%d_%d_end_loop:\n", branch_toggle,
              target_toggle);

      fprintf(fp, "\tret\n");
#endif
    }
  }

  define_gadgets_array(fp, "phr_branch_target_xor_gadgets");
  for (int branch_toggle = min_branch_toggle;
       branch_toggle <= max_branch_toggle; branch_toggle++) {
    for (int target_toggle = min_target_toggle;
         target_toggle <= max_target_toggle; target_toggle++) {
      add_gadget(fp, "phr_branch_target_xor_%d_%d", branch_toggle,
                 target_toggle);
    }
  }
  return 0;
}
