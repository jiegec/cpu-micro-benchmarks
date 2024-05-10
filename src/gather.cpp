#include "include/utils.h"
#include <immintrin.h>
#include <time.h>
#include <unistd.h>

int res = 0;
const int n = 1000;
int array[n] = {0};
const int repeat = 1000;
const int unroll = 8;

void test_1(int *indices) {
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
  }
  res += index[0];
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
  int indices[8];
  srand(time(NULL));
  for (int i = 0; i < 8; i++) {
    indices[i] = rand() % 32;
  }

  printf("Numbers:");
  for (int i = 0; i < 8; i++) {
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

  printf("%ld cycles, %ld instructions, %.2lf ipc, %d ans\n",
         (cycles_after - cycles_before) / m / repeat / unroll,
         (instructions_after - instructions_before) / m / repeat / unroll,
         (double)(instructions_after - instructions_before) /
             (cycles_after - cycles_before),
         res);
  return 0;
}
