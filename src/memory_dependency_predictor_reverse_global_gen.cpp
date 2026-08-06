#include "include/utils.h"

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "w");
  assert(fp);

  // inner functions
  int num_inners = 10;
  // args: ptr1, ptr2
  fprintf(fp, ".text\n");
  for (int inner = 0; inner < num_inners; inner++) {
    fprintf(fp, ".global memory_dependency_predictor_reverse_global_inner_%d\n",
            inner);
    fprintf(fp, ".balign 64\n");
    fprintf(fp, "memory_dependency_predictor_reverse_global_inner_%d:\n",
            inner);
#if defined(HOST_AMD64)
    fprintf(fp, "\tlfence\n");
    // delay store address
    for (int i = 0; i < 20; i++) {
      fprintf(fp, "\timulq $1, %%rdi\n");
    }
    // store
    fprintf(fp, "\tmovq $0x1234, 0(%%rdi)\n");
    // load
    fprintf(fp, "\tmovq 0(%%rsi), %%rcx\n");
    // increase penalty
    for (int i = 0; i < 20; i++) {
      fprintf(fp, "\timulq $1, %%rcx\n");
    }

    fprintf(fp, "\tlfence\n");
    fprintf(fp, "\tret\n");
#endif
  }

  int num_sizes = 100;
  // args: ptr1
  fprintf(fp, ".text\n");
  for (int size = 0; size < num_sizes; size++) {
    fprintf(fp, ".global memory_dependency_predictor_reverse_global_%d\n",
            size);
    fprintf(fp, ".balign 64\n");
    fprintf(fp, "memory_dependency_predictor_reverse_global_%d:\n", size);
#if defined(HOST_AMD64)
    // rdi: buffer
    std::vector<int> gadget_index;
    std::vector<int> allow;
    int counter_max = 4;
    int max_index = 1000 + counter_max + size;
    for (int i = 0; i < max_index; i++) {
      if (size == 0) {
        // special, all allow for warmup
        gadget_index.push_back(i % 10);
        allow.push_back(true);
      } else {
        // first 1000: all allow bypass
        // next counter_max: disallow bypass -> trigger global
        // rest: allow allow bypass
        // last: disallow bypass
        gadget_index.push_back(
            (i < 1000)
                ? i % 10
                : (i < 1000 + counter_max
                       ? i - 1000
                       : ((i == max_index - 1) ? counter_max : counter_max)));
        allow.push_back((i < 1000 || i >= 1000 + counter_max) &&
                        i != (max_index - 1));
      }
    }
    for (int i = 0; i < max_index; i++) {
      // allow or not?
      fprintf(fp, "\tleaq %d(%%rdi), %%rsi\n", allow[i] ? 64 : 0);
      // call
      fprintf(fp,
              "\tcall memory_dependency_predictor_reverse_global_inner_%d\n",
              gadget_index[i]);
    }

    fprintf(fp, "\tret\n");
#endif
  }

  define_gadgets_array(fp,
                       "memory_dependency_predictor_reverse_global_gadgets");
  for (int size = 0; size < num_sizes; size++) {
    add_gadget(fp, "memory_dependency_predictor_reverse_global_%d", size);
  }
  return 0;
}
