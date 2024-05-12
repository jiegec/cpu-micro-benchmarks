#include "include/utils.h"
#include <immintrin.h>
#include <time.h>
#include <unistd.h>

int res = 0;
const int n = 1000;
int array[n] = {0};
const int repeat = 500;
const int unroll = 16;

void test_1(int *indices) {
#ifdef AVX2
  __m256i index = _mm256_loadu_si256((__m256i *)indices);
  for (int i = 0; i < repeat; i++) {
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
    index = _mm256_i32gather_epi32(array, index, 4);
  }
  res += index[0];
#endif
#ifdef AVX512
  __m512i index = _mm512_loadu_si512((__m512i *)indices);
  for (int i = 0; i < repeat; i++) {
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
    index = _mm512_i32gather_epi32(index, array, 4);
  }
  res += index[0];
#endif
}

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
#ifdef AVX2
  const int vlen = 8;
#endif
#ifdef AVX512
  const int vlen = 16;
#endif
  int indices[vlen];
  srand(time(NULL));
  for (int i = 0; i < vlen; i++) {
    indices[i] = rand() % 32;
  }

  printf("Numbers:");
  for (int i = 0; i < vlen; i++) {
    // generate patterns
    printf(" %d", indices[i]);
    array[indices[i]] = indices[i];
  }
  printf("\n");

  int warmup = 1000;

  for (int i = 0; i < warmup; i++) {
    test_1(indices);
  }

  int m = 50000;
  uint64_t cycles_before = perf_read_cycles();
  uint64_t instructions_before = perf_read_instructions();

  for (int i = 0; i < m; i++) {
    test_1(indices);
  }

  uint64_t cycles_after = perf_read_cycles();
  uint64_t instructions_after = perf_read_instructions();

  // i9-14900K: AVX2 24 cycles
  // i9-12900KS: AVX2 24 cycles
  // i9-10980XE: AVX2 38 cycles, AVX512 43 cycles
  // EPYC 9654: AVX2 20 cycles, AVX512 33 cycles
  // EPYC 7742: AVX2 21 cycles
  // EPYC 7551: AVX2 20 cycles
  printf("%ld cycles, %ld instructions, %.2lf ipc, %d ans\n",
         (cycles_after - cycles_before) / m / repeat / unroll,
         (instructions_after - instructions_before) / m / repeat / unroll,
         (double)(instructions_after - instructions_before) /
             (cycles_after - cycles_before),
         res);
  return 0;
}
