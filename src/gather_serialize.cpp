#include "include/utils.h"
#include <assert.h>
#include <cstdint>
#include <time.h>
#include <unistd.h>

#if defined(AVX2) || defined(AVX512)
#include <immintrin.h>
#endif

#ifdef SVE
#include <arm_sve.h>
#endif

int res = 0;
const int n = 8 * 1024 * 1024;
uint64_t array[n] = {0};
uint64_t index_array[n] = {0};
const int repeat = 500;
const int unroll = 16;

void test_1(uint64_t *indices) {
#ifdef AVX2
  __m256i index = _mm256_loadu_si256((__m256i *)indices);
  for (int i = 0; i < repeat; i++) {
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
    index = _mm256_i64gather_epi64((const long long *)array, index, 8);
  }
  res += index[0];
#endif
#ifdef AVX512
  __m512i index = _mm512_loadu_si512((__m512i *)indices);
  for (int i = 0; i < repeat; i++) {
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
    index = _mm512_i64gather_epi64(index, array, 8);
  }
  res += index[0];
#endif
#ifdef SVE
  svbool_t predicate = svwhilelt_b64_s64(0, 8);
  svint64_t index = svld1_s64(predicate, (int64_t *)indices);
  for (int i = 0; i < repeat; i++) {
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
    index = svld1_gather_s64index_s64(predicate, (int64_t *)array, index);
  }
  res += svlastb_s64(predicate, index);
#endif
}

void benchmark(uint64_t *indices) {
  int warmup = 1000;

#ifdef AVX2
  const int vlen = 4;
#endif
#ifdef AVX512
  const int vlen = 8;
#endif
#ifdef SVE
  const int vlen = svlen_s64(svint64_t{});
#endif
  // first ten of first index
  if (0) {
    for (int lane = 0; lane < vlen; lane++) {
      printf("Testing lane %d\n", lane);
      int cur = indices[lane];
      for (int i = 0; i < 10; i++) {
        printf("%d: %d\n", i, cur);
        cur = array[cur];
      }
    }
  }

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

  printf("%.2f cycles, %ld instructions, %.2lf ipc, %d ans\n",
         (float)(cycles_after - cycles_before) / m / repeat / unroll,
         (instructions_after - instructions_before) / m / repeat / unroll,
         (double)(instructions_after - instructions_before) /
             (cycles_after - cycles_before),
         res);
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

#ifdef AVX2
  const int vlen = 4;
#endif
#ifdef AVX512
  const int vlen = 8;
#endif
#ifdef SVE
  const int vlen = svlen_s64(svint64_t{});
#endif
  uint64_t indices[vlen];

  // set array with serialized patterns
  srand(time(NULL));
  // shuffle index, keeping index[0]=0
  for (int i = 0; i < n - vlen; i++) {
    index_array[i] = i;
  }
  for (int i = 1; i < n - vlen; i++) {
    int j = rand() % i;
    int temp = index_array[i];
    index_array[i] = index_array[j];
    index_array[j] = temp;
  }

  // first lane: jumping in array, others are not changing
  // randomize [0, n-vlen)
  // lane 0: 0->...-> pointer chasing
  for (int i = 0; i < n - vlen; i++) {
    array[index_array[i]] = index_array[i + 1];
  }
  array[index_array[n - vlen - 1]] = index_array[0];

  // initialize indices
  indices[0] = 0;
  for (int i = 1; i < vlen; i++) {
    indices[i] = n - vlen + i;
    array[n - vlen + i] = n - vlen + i;
  }

  printf("Testing Serialized\n");
  benchmark(indices);

  // set array with interleaved patterns
  for (int lane = 0; lane < vlen; lane++) {
    for (int i = 0; i < n / vlen / vlen; i++) {
      index_array[i] = i;
    }
    for (int i = 1; i < n / vlen / vlen; i++) {
      int j = rand() % i;
      int temp = index_array[i];
      index_array[i] = index_array[j];
      index_array[j] = temp;
    }

    int offset = n / vlen * lane;
    // array split into: vlen parts
    // each vlen items in one group, shuffle groups lane 0:
    // 0->1->2->3->...->vlen-1->rand1->rand1+1->rand1+2->...->rand1+vlen-1->rand2
    // randomize [offset, offset + n / vlen) in groups
    for (int i = 0; i < n / vlen / vlen; i++) {
      // rand1 -> rand1+1
      for (int j = 0; j < vlen - 1; j++) {
        array[index_array[i] * vlen + offset + j] =
            index_array[i] * vlen + offset + j + 1;
      }

      // rand1+vlen-1->rand2
      if (i == n / vlen - 1) {
        array[index_array[i] * vlen + offset + vlen - 1] =
            index_array[0] * vlen + offset;
      } else {
        array[index_array[i] * vlen + offset + vlen - 1] =
            index_array[i + 1] * vlen + offset;
      }
    }

    // initialize indices
    // first lane: miss #1
    // second lane: miss #2
    indices[lane] = offset + vlen - 1 - lane;
  }

  printf("Testing Interleaved\n");
  benchmark(indices);
  return 0;
}
