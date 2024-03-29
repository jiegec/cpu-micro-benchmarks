#ifdef __aarch64__
INSTR_TEST(unit, "add x0, x0, x0\n", "x0")

INSTR_TEST(int_add, "add x0, x1, x0\n", "x0")
INSTR_TEST(int_add_tp, "add x0, x1, x2\n", "x0")

INSTR_TEST(int_mul, "mul x0, x1, x0\n", "x0")
INSTR_TEST(int_mul_2, "mul x0, x0, x1\n", "x0")
INSTR_TEST(int_mul_3, "mul x8, x9, x8\n", "x8")
INSTR_TEST(int_mul_4, "mul x2, x2, x0\n", "x2")
INSTR_TEST(int_mul_tp, "mul x0, x1, x2\n", "x0")

INSTR_TEST(int_smull, "smull x0, w1, w0\n", "x0")
INSTR_TEST(int_smull_tp, "smull x0, w1, w2\n", "x0")

INSTR_TEST(int_madd, "madd x0, x0, x0, x0\n", "x0")
INSTR_TEST(int_madd_2, "madd x0, x1, x2, x0\n", "x0")
INSTR_TEST(int_madd_3, "madd x0, x1, x0, x2\n", "x0")
INSTR_TEST(int_madd_4, "madd x0, x0, x1, x2\n", "x0")
INSTR_TEST(int_madd_tp, "madd x0, x1, x2, x3\n", "x0")

INSTR_TEST(int_sdiv, "sdiv x0, x1, x0\n", "x0")
INSTR_TEST(int_sdiv_tp, "sdiv x0, x1, x2\n", "x0")

INSTR_TEST(int_udiv, "udiv x0, x1, x0\n", "x0")
INSTR_TEST(int_udiv_tp, "udiv x0, x1, x2\n", "x0")

INSTR_TEST(fp_fadd_single, "fadd s0, s0, s0\n", "s0")
INSTR_TEST(fp_fadd_single_2, "fadd s0, s0, s1\n", "s0")
INSTR_TEST(fp_fadd_single_tp, "fadd s0, s1, s2\n", "s0")

INSTR_TEST(fp_fadd_double, "fadd d0, d0, d0\n", "d0")
INSTR_TEST(fp_fadd_double_2, "fadd d0, d0, d1\n", "d0")
INSTR_TEST(fp_fadd_double_tp, "fadd d0, d1, d2\n", "d0")

INSTR_TEST(fp_fmul_single, "fmul s0, s0, s0\n", "s0")
INSTR_TEST(fp_fmul_single_2, "fmul s0, s0, s1\n", "s0")
INSTR_TEST(fp_fmul_single_tp, "fmul s0, s1, s2\n", "s0")

INSTR_TEST(fp_fmul_double, "fmul d0, d0, d0\n", "d0")
INSTR_TEST(fp_fmul_double_2, "fmul d0, d0, d1\n", "d0")
INSTR_TEST(fp_fmul_double_tp, "fmul d0, d1, d2\n", "d0")

INSTR_TEST(fp_fdiv_single, "fdiv s0, s0, s0\n", "s0")
INSTR_TEST(fp_fdiv_double, "fdiv d0, d0, d0\n", "d0")

INSTR_TEST(fp_fsqrt_single, "fsqrt s0, s0\n", "s0")
INSTR_TEST(fp_fsqrt_double, "fsqrt d0, d0\n", "d0")

INSTR_TEST(fp_fmadd_single, "fmadd s0, s0, s0, s0\n", "v0")
INSTR_TEST(fp_fmadd_single_2, "fmadd s0, s0, s1, s2\n", "v0")
INSTR_TEST(fp_fmadd_single_3, "fmadd s0, s1, s0, s2\n", "v0")
INSTR_TEST(fp_fmadd_single_4, "fmadd s0, s1, s2, s0\n", "v0")
INSTR_TEST(fp_fmadd_single_tp, "fmadd s0, s1, s2, s3\n", "v0")

INSTR_TEST(fp_fmadd_double, "fmadd d0, d0, d0, d0\n", "v0")
INSTR_TEST(fp_fmadd_double_2, "fmadd d0, d0, d1, d2\n", "v0")
INSTR_TEST(fp_fmadd_double_3, "fmadd d0, d1, d0, d2\n", "v0")
INSTR_TEST(fp_fmadd_double_4, "fmadd d0, d1, d2, d0\n", "v0")
INSTR_TEST(fp_fmadd_double_tp, "fmadd d0, d1, d2, d3\n", "v0")

INSTR_TEST(asimd_int_add_half, "add v0.8h, v0.8h, v0.8h\n", "v0")
INSTR_TEST(asimd_int_add_half_2, "add v0.8h, v0.8h, v1.8h\n", "v0")
INSTR_TEST(asimd_int_add_half_tp, "add v0.8h, v1.8h, v2.8h\n", "v0")

INSTR_TEST(asimd_int_add_single, "add v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_int_add_single_2, "add v0.4s, v0.4s, v1.4s\n", "v0")
INSTR_TEST(asimd_int_add_single_tp, "add v0.4s, v1.4s, v2.4s\n", "v0")

INSTR_TEST(asimd_int_add_double, "add v0.2d, v0.2d, v0.2d\n", "v0")
INSTR_TEST(asimd_int_add_double_2, "add v0.2d, v0.2d, v1.2d\n", "v0")
INSTR_TEST(asimd_int_add_double_tp, "add v0.2d, v1.2d, v2.2d\n", "v0")

INSTR_TEST(asimd_int_mul_half, "mul v0.8h, v0.8h, v0.8h\n", "v0")
INSTR_TEST(asimd_int_mul_half_2, "mul v0.8h, v0.8h, v1.8h\n", "v0")

INSTR_TEST(asimd_int_mul_single, "mul v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_int_mul_single_2, "mul v0.4s, v0.4s, v1.4s\n", "v0")
INSTR_TEST(asimd_int_mul_single_tp, "mul v0.4s, v1.4s, v2.4s\n", "v0")

INSTR_TEST(asimd_fp_fmul_single, "fmul v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fp_fmul_single_tp, "fmul v0.4s, v1.4s, v2.4s\n", "v0")

INSTR_TEST(asimd_fp_fmul_double, "fmul v0.2d, v0.2d, v0.2d\n", "d0")
INSTR_TEST(asimd_fp_fmul_double_tp, "fmul v0.2d, v1.2d, v2.2d\n", "d0")

INSTR_TEST(asimd_fp_fadd_single, "fadd v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fp_fadd_single_tp, "fadd v0.4s, v1.4s, v2.4s\n", "v0")

INSTR_TEST(asimd_fp_fadd_double, "fadd v0.2d, v0.2d, v0.2d\n", "d0")
INSTR_TEST(asimd_fp_fadd_double_tp, "fadd v0.2d, v1.2d, v2.2d\n", "d0")

INSTR_TEST(asimd_fp_fmla_single, "fmla v0.4s, v0.4s, v0.4s\n", "v0")
INSTR_TEST(asimd_fp_fmla_single_2, "fmla v0.4s, v1.4s, v2.4s\n", "v0")
INSTR_TEST(asimd_fp_fmla_single_3, "fmla v0.4s, v0.4s, v2.4s\n", "v0")
INSTR_TEST(asimd_fp_fmla_single_4, "fmla v0.4s, v1.4s, v0.4s\n", "v0")

#elif defined(__x86_64__)
INSTR_TEST(unit, "add %%rbx, %%rax\n", "rax")
INSTR_TEST(int_add, "add %%rbx, %%rax\n", "rax")
INSTR_TEST(int_andn, "andn %%rbx, %%rax, %%rax\n", "rax")
INSTR_TEST(int_andn_tp, "andn %%rbx, %%rcx, %%rax\n", "rax")
INSTR_TEST(int_lea_add, "lea (%%rbx,%%rax), %%rax\n", "rax")
INSTR_TEST(int_lea_add_tp, "lea (%%rbx,%%rcx), %%rax\n", "rax")
INSTR_TEST(sse_addsd, "addsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_addpd, "addpd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_subsd, "subsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_subpd, "subpd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_mulsd, "mulsd %%xmm0, %%xmm1\n", "xmm0")
INSTR_TEST(sse_mulpd, "mulpd %%xmm0, %%xmm1\n", "xmm0")

#elif defined(__powerpc__)
INSTR_TEST(int_add, "add %%r0, %%r1, %%r0\n", "r0")
INSTR_TEST(int_mulhw, "mulhw %%r0, %%r1, %%r0\n", "r0")
INSTR_TEST(int_mulld, "mulld %%r0, %%r1, %%r0\n", "r0")

#elif defined(__loongarch__)
INSTR_TEST(unit, "add.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_add32, "add.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_add32_tp, "add.w $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_add64, "add.d $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_add64_tp, "add.d $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_mul32, "mul.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_mul32_tp, "mul.w $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_mul64, "mul.d $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_mul64_tp, "mul.d $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_div64, "div.d $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crc_w_b_w, "crc.w.b.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crc_w_b_w_tp, "crc.w.b.w $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_crc_w_h_w, "crc.w.h.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crc_w_h_w_tp, "crc.w.h.w $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_crc_w_w_w, "crc.w.w.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crc_w_w_w_tp, "crc.w.w.w $r12, $r1, $r2\n", "r12")
INSTR_TEST(int_crc_w_d_w, "crc.w.d.w $r12, $r1, $r12\n", "r12")
INSTR_TEST(int_crc_w_d_w_tp, "crc.w.d.w $r12, $r1, $r2\n", "r12")
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
INSTR_TEST(fp_fmadd_s_3, "fmadd.s $f0, $f1, $f2, $f0\n", "f0")
INSTR_TEST(fp_fmadd_d_3, "fmadd.d $f0, $f1, $f2, $f0\n", "f0")
INSTR_TEST(fp_fmadd_s_2, "fmadd.s $f0, $f1, $f0, $f3\n", "f0")
INSTR_TEST(fp_fmadd_d_2, "fmadd.d $f0, $f1, $f0, $f3\n", "f0")
INSTR_TEST(fp_fmadd_s_1, "fmadd.s $f0, $f0, $f2, $f3\n", "f0")
INSTR_TEST(fp_fmadd_d_1, "fmadd.d $f0, $f0, $f2, $f3\n", "f0")
INSTR_TEST(fp_fmadd_s_tp, "fmadd.s $f0, $f1, $f2, $f3\n", "f0")
INSTR_TEST(fp_fmadd_d_tp, "fmadd.d $f0, $f1, $f2, $f3\n", "f0")

INSTR_TEST(lsx_int_vadd_d, "vadd.d $vr0, $vr1, $vr0\n")
INSTR_TEST(lsx_int_vadd_d_tp, "vadd.d $vr0, $vr1, $vr2\n")

INSTR_TEST(lsx_int_vmul_d, "vmul.d $vr0, $vr1, $vr0\n")
INSTR_TEST(lsx_int_vmul_d_tp, "vmul.d $vr0, $vr1, $vr2\n")

INSTR_TEST(lsx_fp_vfadd_d, "vfadd.d $vr0, $vr1, $vr0\n")
INSTR_TEST(lsx_fp_vfadd_d_tp, "vfadd.d $vr0, $vr1, $vr2\n")

INSTR_TEST(lsx_fp_vfmul_d, "vfmul.d $vr0, $vr1, $vr0\n")
INSTR_TEST(lsx_fp_vfmul_d_tp, "vfmul.d $vr0, $vr1, $vr2\n")

INSTR_TEST(lsx_fp_vfmadd_d, "vfmadd.d $vr0, $vr1, $vr2, $vr0\n")
INSTR_TEST(lsx_fp_vfmadd_d_2, "vfmadd.d $vr0, $vr1, $vr0, $vr3\n")
INSTR_TEST(lsx_fp_vfmadd_d_3, "vfmadd.d $vr0, $vr0, $vr2, $vr3\n")
INSTR_TEST(lsx_fp_vfmadd_d_tp, "vfmadd.d $vr0, $vr1, $vr2, $vr3\n")

INSTR_TEST(lasx_int_xvadd_d, "xvadd.d $xr0, $xr1, $xr0\n")
INSTR_TEST(lasx_int_xvadd_d_tp, "xvadd.d $xr0, $xr1, $xr2\n")

INSTR_TEST(lasx_int_xvmul_d, "xvmul.d $xr0, $xr1, $xr0\n")
INSTR_TEST(lasx_int_xvmul_d_tp, "xvmul.d $xr0, $xr1, $xr2\n")

INSTR_TEST(lasx_fp_xvfadd_d, "xvfadd.d $xr0, $xr1, $xr0\n")
INSTR_TEST(lasx_fp_xvfadd_d_tp, "xvfadd.d $xr0, $xr1, $xr2\n")

INSTR_TEST(lasx_fp_xvfmul_d, "xvfmul.d $xr0, $xr1, $xr0\n")
INSTR_TEST(lasx_fp_xvfmul_d_tp, "xvfmul.d $xr0, $xr1, $xr2\n")

INSTR_TEST(lasx_fp_xvfmadd_d, "xvfmadd.d $xr0, $xr1, $xr2, $xr0\n")
INSTR_TEST(lasx_fp_xvfmadd_d_2, "xvfmadd.d $xr0, $xr1, $xr0, $xr3\n")
INSTR_TEST(lasx_fp_xvfmadd_d_3, "xvfmadd.d $xr0, $xr0, $xr2, $xr3\n")
INSTR_TEST(lasx_fp_xvfmadd_d_tp, "xvfmadd.d $xr0, $xr1, $xr2, $xr3\n")

#endif
