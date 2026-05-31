#include <stdint.h>

typedef uint32_t IndexType;

#define N 0x1000

void transform_c(int32_t *__restrict__ output,
                 const int8_t *__restrict__ weights,
                 const int32_t *__restrict__ biases,
                 const uint8_t *__restrict__ input) {
  for (IndexType i = 0; i < N; ++i) {
    const IndexType offset = i * N;
    int32_t sum = biases[i];
    for (IndexType j = 0; j < N; ++j) {
      sum += (int32_t)weights[offset + j] * (int32_t)input[j];
    }
    output[i] = sum;
  }
}
