#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct vfloat4 {
  float m[4];
};

/* All implementations have the same signature:
   struct vfloat4 vfmax_*(struct vfloat4 a, struct vfloat4 b) */
extern struct vfloat4 vfmax_c(struct vfloat4, struct vfloat4);
extern struct vfloat4 vfmax_gcc16(struct vfloat4, struct vfloat4);
extern struct vfloat4 vfmax_clang22(struct vfloat4, struct vfloat4);
extern struct vfloat4 vfmax_opt(struct vfloat4, struct vfloat4);

/* Number of vfloat4 pairs to test */
#define N 1000000

static uint32_t f32_bits(float x) {
  uint32_t u;
  __builtin_memcpy(&u, &x, sizeof(u));
  return u;
}

static int validate_bit(const char *name, const struct vfloat4 *ref,
                        const struct vfloat4 *test, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < 4; j++) {
      uint32_t r = f32_bits(ref[i].m[j]);
      uint32_t t = f32_bits(test[i].m[j]);
      if (r != t) {
        fprintf(stderr,
                "MISMATCH: %s at index %d elem %d: "
                "ref=0x%08x (%.20g) test=0x%08x (%.20g)\n",
                name, i, j, r, ref[i].m[j], t, test[i].m[j]);
        return 1;
      }
    }
  }
  return 0;
}

static double bench(const char *name,
                    struct vfloat4 (*func)(struct vfloat4, struct vfloat4),
                    const struct vfloat4 *a, const struct vfloat4 *b,
                    struct vfloat4 *out, int n, int iterations) {
  /* Warm-up */
  for (int i = 0; i < n; i++)
    out[i] = func(a[i], b[i]);

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int iter = 0; iter < iterations; iter++)
    for (int i = 0; i < n; i++)
      out[i] = func(a[i], b[i]);
  clock_gettime(CLOCK_MONOTONIC, &t1);

  double total_ns = (t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec);
  double per_call_ns = total_ns / (double)(iterations * n);
  double million_ops = (double)(iterations * n) / total_ns * 1000.0;

  printf("%-18s %9.2f  %8.0f    %d pairs x %d iters\n", name, per_call_ns,
         million_ops, n, iterations);
  return per_call_ns;
}

int main(void) {
  int n = 100000;
  int iterations = 100;

  struct vfloat4 *a = aligned_alloc(16, n * sizeof(struct vfloat4));
  struct vfloat4 *b = aligned_alloc(16, n * sizeof(struct vfloat4));
  struct vfloat4 *ref = aligned_alloc(16, n * sizeof(struct vfloat4));
  struct vfloat4 *t_gcc16 = aligned_alloc(16, n * sizeof(struct vfloat4));
  struct vfloat4 *t_clang22 = aligned_alloc(16, n * sizeof(struct vfloat4));
  struct vfloat4 *t_opt = aligned_alloc(16, n * sizeof(struct vfloat4));

  if (!a || !b || !ref || !t_gcc16 || !t_clang22 || !t_opt) {
    fprintf(stderr, "aligned_alloc failed\n");
    return 1;
  }

  /* Deterministic input — mix random floats with NaN, inf, -inf */
  static const float specials[] = {
      0.0f / 0.0f,  /* NaN */
      -0.0f / 0.0f, /* -NaN */
      1.0f / 0.0f,  /* +inf */
      -1.0f / 0.0f, /* -inf */
  };
  srand(42);
  for (int i = 0; i < n; i++)
    for (int j = 0; j < 4; j++) {
      float v = (float)(rand() % 20000 - 10000) / 10.0f;
      /* ~8% chance of a special value */
      int r = rand() % 100;
      if (r < 3)
        v = specials[0]; /* NaN */
      else if (r < 5)
        v = specials[1]; /* -NaN */
      else if (r < 6)
        v = specials[2]; /* +inf */
      else if (r < 8)
        v = specials[3]; /* -inf */
      a[i].m[j] = v;
      v = (float)(rand() % 20000 - 10000) / 10.0f;
      r = rand() % 100;
      if (r < 3)
        v = specials[0];
      else if (r < 5)
        v = specials[1];
      else if (r < 6)
        v = specials[2];
      else if (r < 8)
        v = specials[3];
      b[i].m[j] = v;
    }

  printf("=== Correctness validation ===\n");

  /* Compute reference output */
  for (int i = 0; i < n; i++)
    ref[i] = vfmax_c(a[i], b[i]);

  /* Validate each implementation */
  for (int i = 0; i < n; i++)
    t_gcc16[i] = vfmax_gcc16(a[i], b[i]);
  for (int i = 0; i < n; i++)
    t_clang22[i] = vfmax_clang22(a[i], b[i]);
  for (int i = 0; i < n; i++)
    t_opt[i] = vfmax_opt(a[i], b[i]);

  int errors = 0;
  errors += validate_bit("vfmax_gcc16", ref, t_gcc16, n);
  errors += validate_bit("vfmax_clang22", ref, t_clang22, n);
  errors += validate_bit("vfmax_opt", ref, t_opt, n);

  if (errors) {
    printf("\nVALIDATION FAILED with %d error(s)\n", errors);
    free(a);
    free(b);
    free(ref);
    free(t_gcc16);
    free(t_clang22);
    free(t_opt);
    return 1;
  }
  printf("All implementations match the C reference.\n\n");

  printf("=== Performance benchmark ===\n");
  printf("%-18s %9s  %8s\n", "Variant", "ns/op", "M vfmax/s");
  printf("--------------------------------------\n");

  bench("ref (vfmax_c)", vfmax_c, a, b, ref, n, iterations);
  bench("gcc16 (vfmax_gcc16)", vfmax_gcc16, a, b, t_gcc16, n, iterations);
  bench("clang (vfmax_clang)", vfmax_clang22, a, b, t_clang22, n, iterations);
  bench("opt (vfmax_opt)", vfmax_opt, a, b, t_opt, n, iterations);

  free(a);
  free(b);
  free(ref);
  free(t_gcc16);
  free(t_clang22);
  free(t_opt);
  return 0;
}
