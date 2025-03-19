#include "include/utils.h"
#include <arm_neon.h>
#include <arm_sve.h>
#include <time.h>
#include <unistd.h>

int res = 0;
const int n = 4000;
uint32_t array[n] = {0};
const int repeat = 1200;
const int unroll = 16;

#ifdef SVE_FP32_ADD
void test1(float *indices) {
  float tmp[svcntw()];
  svbool_t pg = svptrue_b32();
  svfloat32_t v0 = svdup_f32(1.0);
  svfloat32_t v1 = svld1_f32(pg, indices);
  for (int i = 0; i < n; i++) {
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
    v1 = svadd_f32_z(pg, v1, v0);
  }
  svst1_f32(pg, tmp, v1);
  res += tmp[0];
}
#endif

#ifdef NEON_FP32_ADD
void test1(float *indices) {
  float tmp[4];
  float32x4_t v0 = vdupq_n_f32(1.0);
  float32x4_t v1 = vld1q_f32(indices);
  for (int i = 0; i < n; i++) {
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
    v1 = vaddq_f32(v1, v0);
  }
  vst1q_f32(tmp, v1);
  res += tmp[0];
}
#endif

#ifdef SVE_FP64_ADD
void test1(double *indices) {
  double tmp[svcntd()];
  svbool_t pg = svptrue_b64();
  svfloat64_t v0 = svdup_f64(1.0);
  svfloat64_t v1 = svld1_f64(pg, indices);
  for (int i = 0; i < n; i++) {
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
    v1 = svadd_f64_z(pg, v1, v0);
  }
  svst1_f64(pg, tmp, v1);
  res += tmp[0];
}
#endif

#ifdef NEON_FP64_ADD
void test1(double *indices) {
  double tmp[2];
  float64x2_t v0 = vdupq_n_f64(1.0);
  float64x2_t v1 = vld1q_f64(indices);
  for (int i = 0; i < n; i++) {
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
    v1 = vaddq_f64(v1, v0);
  }
  vst1q_f64(tmp, v1);
  res += tmp[0];
}
#endif

#ifdef SVE_FP32_FMA
void test1(float *indices) {
  float tmp[svcntw()];
  svbool_t pg = svptrue_b32();
  svfloat32_t v0 = svdup_f32(1.0);
  svfloat32_t v1 = svld1_f32(pg, indices);
  for (int i = 0; i < n; i++) {
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f32_z(pg, v1, v0, 1.0);
  }
  svst1_f32(pg, tmp, v1);
  res += tmp[0];
}
#endif

#ifdef NEON_FP32_FMA
void test1(float *indices) {
  float tmp[4];
  float32x4_t v0 = vdupq_n_f32(1.0);
  float32x4_t v1 = vld1q_f32(indices);
  for (int i = 0; i < n; i++) {
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
    v1 = vfmaq_n_f32(v1, v0, 1.0);
  }
  vst1q_f32(tmp, v1);
  res += tmp[0];
}
#endif

#ifdef SVE_FP64_FMA
void test1(double *indices) {
  double tmp[svcntd()];
  svbool_t pg = svptrue_b64();
  svfloat64_t v0 = svdup_f64(1.0);
  svfloat64_t v1 = svld1_f64(pg, indices);
  for (int i = 0; i < n; i++) {
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
    v1 = svmla_n_f64_z(pg, v1, v0, 1.0);
  }
  svst1_f64(pg, tmp, v1);
  res += tmp[0];
}
#endif

#ifdef NEON_FP64_FMA
void test1(double *indices) {
  double tmp[2];
  float64x2_t v0 = vdupq_n_f64(1.0);
  float64x2_t v1 = vld1q_f64(indices);
  for (int i = 0; i < n; i++) {
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
    v1 = vfmaq_n_f64(v1, v0, 1.0);
  }
  vst1q_f64(tmp, v1);
  res += tmp[0];
}
#endif

int main(int argc, char *argv[]) {

  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
    switch (opt) {
    default:
      fprintf(stderr, "Usage: %s [-p]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  bind_to_core();
  setup_perf_instructions();
  setup_perf_cycles();

  // int indices[] = {0, 1, 2, 3, 4, 5, 6, 7};
#if defined(NEON_FP32_ADD) || defined(NEON_FP32_FMA)
  const int vlen = 4;
  float indices[vlen];
#endif

#if defined(NEON_FP64_ADD) || defined(NEON_FP64_FMA)
  const int vlen = 2;
  double indices[vlen];
#endif

#if defined(SVE_FP32_ADD) || defined(SVE_FP32_FMA)
  const int vlen = svcntw();
  float indices[vlen];
#endif

#if defined(SVE_FP64_ADD) || defined(SVE_FP64_FMA)
  const int vlen = svcntd();
  double indices[vlen];
#endif
  for (int i = 0; i < vlen; i++) {
    indices[i] = i + 1.0;
  }

  // printf("Numbers:");
  // for (int i = 0; i < vlen; i++) {
  //   // generate patterns
  //   printf(" %d", indices[i]);
  //   array[indices[i]] = indices[i];
  // }
  // printf("\n");

  int warmup = 1000;

  for (int i = 0; i < warmup; i++) {
    test1(indices);
  }

  int m = 50000;
  uint64_t cycles_before = perf_read_cycles();
  uint64_t instructions_before = perf_read_instructions();

  for (int i = 0; i < m; i++) {
    test1(indices);
  }

  uint64_t cycles_after = perf_read_cycles();
  uint64_t instructions_after = perf_read_instructions();

  printf("%ld cycles, %ld instructions, %.2lf ipc, %d ans\n",
         (cycles_after - cycles_before) / m / repeat / unroll,
         (instructions_after - instructions_before) / m / repeat / unroll,
         (double)(instructions_after - instructions_before) /
             (cycles_after - cycles_before),
         res);
  return 0;
}
