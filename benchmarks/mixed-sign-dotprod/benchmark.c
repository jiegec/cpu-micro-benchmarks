#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void transform_c(int32_t *output, const int8_t *weights, const int32_t *biases,
                 const uint8_t *input);
void transform_gcc16(int32_t *output, const int8_t *weights,
                     const int32_t *biases, const uint8_t *input);
void transform_gcc15(int32_t *output, const int8_t *weights,
                     const int32_t *biases, const uint8_t *input);
void transform_opt(int32_t *output, const int8_t *weights,
                   const int32_t *biases, const uint8_t *input);
void transform_clang22(int32_t *output, const int8_t *weights,
                       const int32_t *biases, const uint8_t *input);

#ifdef __cplusplus
}
#endif

#define N 0x1000

static int compare_outputs(const int32_t *a, const int32_t *b,
                           const char *name_a, const char *name_b) {
  for (uint32_t i = 0; i < N; ++i) {
    if (a[i] != b[i]) {
      fprintf(stderr, "MISMATCH at [%u]: %s=%d, %s=%d\n", i, name_a, a[i],
              name_b, b[i]);
      return 0;
    }
  }
  return 1;
}

static double get_time_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

static double benchmark(const char *name,
                        void (*func)(int32_t *, const int8_t *, const int32_t *,
                                     const uint8_t *),
                        int32_t *output, const int8_t *weights,
                        const int32_t *biases, const uint8_t *input,
                        int iterations) {
  /* warm-up */
  for (int i = 0; i < 3; ++i)
    func(output, weights, biases, input);

  double start = get_time_ns();
  for (int i = 0; i < iterations; ++i)
    func(output, weights, biases, input);
  double end = get_time_ns();

  double elapsed_ns = (end - start) / iterations;
  printf("  %-16s %9.0f ns/iter  (%d iters)\n", name, elapsed_ns, iterations);
  return elapsed_ns;
}

int main(void) {
  int8_t *weights;
  int32_t *biases;
  uint8_t *input;
  int32_t *out_c, *out_gcc16, *out_gcc15, *out_opt, *out_clang22;

  /* Aligned allocations: the assembly may read 32-byte vectors */
  posix_memalign((void **)&weights, 32, (size_t)N * N * sizeof(int8_t));
  posix_memalign((void **)&biases, 32, (size_t)N * sizeof(int32_t));
  posix_memalign((void **)&input, 32, (size_t)N * sizeof(uint8_t));
  posix_memalign((void **)&out_c, 32, (size_t)N * sizeof(int32_t));
  posix_memalign((void **)&out_gcc16, 32, (size_t)N * sizeof(int32_t));
  posix_memalign((void **)&out_gcc15, 32, (size_t)N * sizeof(int32_t));
  posix_memalign((void **)&out_opt, 32, (size_t)N * sizeof(int32_t));
  posix_memalign((void **)&out_clang22, 32, (size_t)N * sizeof(int32_t));

  if (!weights || !biases || !input || !out_c || !out_gcc16 || !out_gcc15 ||
      !out_opt || !out_clang22) {
    fprintf(stderr, "allocation failed\n");
    return 1;
  }

  /* Seed random data */
  srand(42);
  for (uint32_t i = 0; i < (uint32_t)N * N; ++i)
    weights[i] = (int8_t)(rand() & 0xff);
  for (uint32_t i = 0; i < N; ++i)
    biases[i] = (int32_t)(rand() & 0xffff);
  for (uint32_t i = 0; i < N; ++i)
    input[i] = (uint8_t)(rand() & 0xff);

  puts("=== Validation ===");
  transform_c(out_c, weights, biases, input);
  transform_gcc16(out_gcc16, weights, biases, input);
  transform_gcc15(out_gcc15, weights, biases, input);
  transform_opt(out_opt, weights, biases, input);
  transform_clang22(out_clang22, weights, biases, input);

  int ok = 1;
  ok = ok && compare_outputs(out_c, out_gcc16, "c", "gcc16");
  ok = ok && compare_outputs(out_c, out_gcc15, "c", "gcc15");
  ok = ok && compare_outputs(out_c, out_opt, "c", "opt");
  ok = ok && compare_outputs(out_c, out_clang22, "c", "clang22");
  if (ok)
    puts("  All implementations produce identical results.\n");
  else
    puts("  MISMATCH detected!\n");

  puts("=== Benchmark (ns per call) ===");
  /* Pick iteration count so it runs for a few seconds */
  int iters = 50;
  benchmark("C (ref)", transform_c, out_c, weights, biases, input, iters);
  benchmark("GCC 16 (LASX)", transform_gcc16, out_gcc16, weights, biases, input,
            iters);
  benchmark("GCC 15 (LASX)", transform_gcc15, out_gcc15, weights, biases, input,
            iters);
  benchmark("OPT (LASX)", transform_opt, out_opt, weights, biases, input,
            iters);
  benchmark("Clang 22 (LSX)", transform_clang22, out_clang22, weights, biases,
            input, iters);

  free(weights);
  free(biases);
  free(input);
  free(out_c);
  free(out_gcc16);
  free(out_gcc15);
  free(out_opt);
  free(out_clang22);
  return ok ? 0 : 1;
}
