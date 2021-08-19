#ifdef __aarch64__
INSTR_TEST(int_add, "add x0, x1, x0\n", "x0")
INSTR_TEST(int_mul, "mul x0, x1, x0\n", "x0")
INSTR_TEST(int_smull, "smull x0, w1, w0\n", "x0")
INSTR_TEST(int_madd, "madd x0, x0, x0, x0\n", "x0")
INSTR_TEST(int_madd2, "madd x0, x1, x2, x0\n", "x0")
INSTR_TEST(int_madd3, "madd x0, x1, x0, x2\n", "x0")
INSTR_TEST(int_madd4, "madd x0, x0, x1, x2\n", "x0")

INSTR_TEST(fp_fmul_single, "fmul s0, s0, s0\n", "s0")
INSTR_TEST(fp_fmul_double, "fmul d0, d0, d0\n", "d0")
INSTR_TEST(fp_fadd_single, "fadd s0, s0, s0\n", "s0")
INSTR_TEST(fp_fadd_double, "fadd d0, d0, d0\n", "d0")

INSTR_TEST(fp_fmadd_single, "fmadd s0, s0, s0, s0\n", "v0")
INSTR_TEST(fp_fmadd2_single, "fmadd s0, s0, s1, s2\n", "v0")
INSTR_TEST(fp_fmadd3_single, "fmadd s0, s1, s0, s2\n", "v0")
INSTR_TEST(fp_fmadd4_single, "fmadd s0, s1, s2, s0\n", "v0")

INSTR_TEST(asimd_fmul_single, "fmul v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fmul_double, "fmul v0.2d, v0.2d, v0.2d\n", "d0")

INSTR_TEST(asimd_fadd_single, "fadd v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fadd_double, "fadd v0.2d, v0.2d, v0.2d\n", "d0")

INSTR_TEST(asimd_fmla_single, "fmla v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fmla2_single, "fmla v0.4s, v1.4s, v2.4s\n", "v0")

#elif defined(__x86_64__)
INSTR_TEST(int_add, "inc %%rax\n", "rax")
INSTR_TEST(sse_addsd, "addsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_subsd, "subsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_mulsd, "mulsd %%xmm0, %%xmm1\n", "xmm0")

#elif defined(__powerpc__)
INSTR_TEST(int_add, "add %%r0, %%r1, %%r0\n", "r0")
INSTR_TEST(int_mulhw, "mulhw %%r0, %%r1, %%r0\n", "r0")
INSTR_TEST(int_mulld, "mulld %%r0, %%r1, %%r0\n", "r0")
#endif
