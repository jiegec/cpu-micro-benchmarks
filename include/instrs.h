#ifdef __aarch64__
INSTR_TEST(add, "add x0, x1, x0\n", "x0")
INSTR_TEST(fmul_single, "fmul s0, s0, s0\n", "s0")
INSTR_TEST(fmul_double, "fmul d0, d0, d0\n", "d0")

#elif defined(__x86_64__)
INSTR_TEST(add, "inc %%rax\n", "rax")
INSTR_TEST(addsd, "addsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(subsd, "subsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(mulsd, "mulsd %%xmm0, %%xmm1\n", "xmm0")
#endif
