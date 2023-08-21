#include "include/utils.h"
#include <assert.h>
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <utility>
#include <vector>

int N = 1000000;

#define INSTR_TEST(NAME, INST, ...)                                            \
  void test_##NAME(int n) {                                                    \
    for (int i = 0; i < n; i++) {                                              \
      asm volatile(HUNDRED(INST) : : : __VA_ARGS__);                           \
    }                                                                          \
  }

#include "include/instrs.h"

#undef INSTR_TEST

struct InstrTest {
  const char *name;
  const char *inst;
  void (*test)(int);
};

#define INSTR_TEST(NAME, INST, ...)                                            \
  InstrTest{.name = #NAME, .inst = #INST, .test = test_##NAME},

std::vector<InstrTest> tests = {
#include "include/instrs.h"
};

#undef INSTR_TEST

struct InstrInfo {
  std::set<double> latency;
  double throughput;
};

int main(int argc, char *argv[]) {
  bool perf = false;

  int opt;
  while ((opt = getopt(argc, argv, "n:p")) != -1) {
    switch (opt) {
    case 'n':
      sscanf(optarg, "%d", &N);
      break;
    case 'p':
      perf = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [-p]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  bind_to_core();
  if (perf) {
    setup_time_or_cycles();
  }

  // calibrate unit cycle time
  uint64_t begin = get_time_or_cycles();
  test_unit(N);
  uint64_t unit_elapsed = get_time_or_cycles() - begin;

  std::map<std::string, InstrInfo> info;

  for (auto it : tests) {
    std::string name = it.name;
    if (name == "unit") {
      continue;
    }

    begin = get_time_or_cycles();
    it.test(N);
    uint64_t elapsed = get_time_or_cycles() - begin;
    double cycles = (double)elapsed / unit_elapsed;

    std::string base_name;
    size_t tp_index = name.find("_tp");
    if (tp_index != std::string::npos) {
      base_name = name.substr(0, tp_index);
      printf("%s: throughput 1/%.2lf=%.2lf instructions\n", it.name, cycles,
             1.0 / cycles);
      info[base_name].throughput = cycles;
    } else {
      base_name = name;

      size_t last_underscore_index = name.find_last_of("_");
      if (last_underscore_index != std::string::npos) {
        char *end = NULL;
        const char *start = &name.c_str()[last_underscore_index + 1];
        strtol(start, &end, 10);

        if (start != end) {
          // strip suffix _2 etc
          base_name = name.substr(0, last_underscore_index);
        }
      }

      // round to 0.01
      cycles = (double)(long)(cycles * 100) / 100.0;
      printf("%s: latency %.2lf cycles\n", it.name, cycles);
      info[base_name].latency.insert(cycles);
    }
  }

  FILE *fp = fopen("instruction_latency.csv", "w");
  assert(fp);
  fprintf(fp, "name,latency,throughput\n");
  for (auto pair : info) {
    std::string latency;
    auto entry = pair.second;
    for (auto lat : entry.latency) {
      char buffer[32];
      std::sprintf(buffer, "%.2lf", lat);
      if (!latency.empty()) {
        latency += "/";
      }
      latency += buffer;
    }
    fprintf(fp, "%s,%s,%.2lf\n", pair.first.c_str(), latency.c_str(),
            entry.throughput);
  }
  printf("Result written to instruction_latency.csv\n");
}
