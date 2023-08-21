#ifdef __aarch64__
INSTR_TEST(int_add, "add x0, x1, x0\n", "x0")
INSTR_TEST(int_add_tp, "add x0, x1, x2\n", "x0")
INSTR_TEST(int_mul, "mul x0, x1, x0\n", "x0")
INSTR_TEST(int_mul_tp, "mul x0, x1, x2\n", "x0")
INSTR_TEST(int_smull, "smull x0, w1, w0\n", "x0")
INSTR_TEST(int_madd, "madd x0, x0, x0, x0\n", "x0")
INSTR_TEST(int_madd2, "madd x0, x1, x2, x0\n", "x0")
INSTR_TEST(int_madd3, "madd x0, x1, x0, x2\n", "x0")
INSTR_TEST(int_madd4, "madd x0, x0, x1, x2\n", "x0")
INSTR_TEST(int_sdiv, "sdiv x0, x1, x0\n", "x0")
INSTR_TEST(int_sdiv_tp, "sdiv x0, x1, x2\n", "x0")
INSTR_TEST(int_udiv, "udiv x0, x1, x0\n", "x0")

INSTR_TEST(fp_fmul_single, "fmul s0, s0, s0\n", "s0")
INSTR_TEST(fp_fmul_double, "fmul d0, d0, d0\n", "d0")
INSTR_TEST(fp_fmul_single_tp, "fmul s0, s1, s2\n", "s0")
INSTR_TEST(fp_fadd_single, "fadd s0, s0, s0\n", "s0")
INSTR_TEST(fp_fadd_double, "fadd d0, d0, d0\n", "d0")
INSTR_TEST(fp_fdiv_single, "fdiv s0, s0, s0\n", "s0")
INSTR_TEST(fp_fdiv_double, "fdiv d0, d0, d0\n", "d0")
INSTR_TEST(fp_fsqrt_single, "fsqrt s0, s0\n", "s0")
INSTR_TEST(fp_fsqrt_double, "fsqrt d0, d0\n", "d0")

INSTR_TEST(fp_fmadd_single, "fmadd s0, s0, s0, s0\n", "v0")
INSTR_TEST(fp_fmadd2_single, "fmadd s0, s0, s1, s2\n", "v0")
INSTR_TEST(fp_fmadd3_single, "fmadd s0, s1, s0, s2\n", "v0")
INSTR_TEST(fp_fmadd4_single, "fmadd s0, s1, s2, s0\n", "v0")

INSTR_TEST(asimd_int_add_half, "add v0.8h, v0.8h, v0.8h\n", "v0")
INSTR_TEST(asimd_int_add_single, "add v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_int_add_double, "add v0.2d, v0.2d, v0.2d\n", "d0")
INSTR_TEST(asimd_int_mul_half, "mul v0.8h, v0.8h, v0.8h\n", "v0")
INSTR_TEST(asimd_int_mul_single, "mul v0.4s, v0.4s, v0.4s\n", "v0")

INSTR_TEST(asimd_fp_fmul_single, "fmul v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fp_fmul_double, "fmul v0.2d, v0.2d, v0.2d\n", "d0")
INSTR_TEST(asimd_fp_fmul_single_tp, "fmul v0.4s, v1.4s, v2.4s\n", "v0")

INSTR_TEST(asimd_fp_fadd_single, "fadd v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fp_fadd_double, "fadd v0.2d, v0.2d, v0.2d\n", "d0")

INSTR_TEST(asimd_fp_fmla_single, "fmla v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fp_fmla2_single, "fmla v0.4s, v1.4s, v2.4s\n", "v0")

#elif defined(__x86_64__)
INSTR_TEST(int_add, "inc %%rax\n", "rax")
INSTR_TEST(sse_addsd, "addsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_subsd, "subsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_mulsd, "mulsd %%xmm0, %%xmm1\n", "xmm0")

#elif defined(__powerpc__)
INSTR_TEST(int_add, "add %%r0, %%r1, %%r0\n", "r0")
INSTR_TEST(int_mulhw, "mulhw %%r0, %%r1, %%r0\n", "r0")
INSTR_TEST(int_mulld, "mulld %%r0, %%r1, %%r0\n", "r0")

#elif defined(__loongarch__)
INSTR_TEST(int_add, "add.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_add32, "add.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_add32_tp, "add.w $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_add64, "add.d $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_add64_tp, "add.d $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_mul32, "mul.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_mul32_tp, "mul.w $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_mul64, "mul.d $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_mul64_tp, "mul.d $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_div64, "div.d $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crc_w_d_w, "crc.w.d.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crc_w_w_w, "crc.w.w.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crcc_w_d_w, "crcc.w.d.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crcc_w_w_w, "crcc.w.w.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(fp_fadd_s, "fadd.s $f0, $f1, $f0\n", "f0")
INSTR_TEST(fp_fadd_s_tp, "fadd.s $f0, $f1, $f2\n", "f0")
INSTR_TEST(fp_fadd_d, "fadd.d $f0, $f1, $f0\n", "f0")
INSTR_TEST(fp_fadd_d_tp, "fadd.d $f0, $f1, $f2\n", "f0")
INSTR_TEST(fp_fmul_s, "fmul.s $f0, $f1, $f0\n", "f0")
INSTR_TEST(fp_fmul_s_tp, "fmul.s $f0, $f1, $f2\n", "f0")
INSTR_TEST(fp_fmul_d, "fmul.d $f0, $f1, $f0\n", "f0")
INSTR_TEST(fp_fmul_d_tp, "fmul.d $f0, $f1, $f2\n", "f0")
INSTR_TEST(fp_fmadd3_s, "fmadd.s $f0, $f1, $f2, $f0\n", "f0")
INSTR_TEST(fp_fmadd3_d, "fmadd.d $f0, $f1, $f2, $f0\n", "f0")
INSTR_TEST(fp_fmadd2_s, "fmadd.s $f0, $f1, $f0, $f3\n", "f0")
INSTR_TEST(fp_fmadd2_d, "fmadd.d $f0, $f1, $f0, $f3\n", "f0")
INSTR_TEST(fp_fmadd1_s, "fmadd.s $f0, $f0, $f2, $f3\n", "f0")
INSTR_TEST(fp_fmadd1_d, "fmadd.d $f0, $f0, $f2, $f3\n", "f0")
INSTR_TEST(fp_fmadd_s_tp, "fmadd.s $f0, $f1, $f2, $f3\n", "f0")
INSTR_TEST(fp_fmadd_d_tp, "fmadd.d $f0, $f1, $f2, $f3\n", "f0")
#endif
