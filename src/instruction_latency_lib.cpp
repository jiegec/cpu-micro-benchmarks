#include "include/utils.h"
#include <assert.h>
#include <set>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#define INSTR_TEST(NAME, INST, ...)                                            \
  void test_##NAME(int n) {                                                    \
    for (int i = 0; i < n; i++) {                                              \
      asm volatile(".align 4\n" THOUSAND(INST) : : : __VA_ARGS__);             \
    }                                                                          \
  }

#define INSTR_TEST_REPEAT(NAME, REPEAT, INST, ...)                             \
  INSTR_TEST(NAME, INST, __VA_ARGS__)
#include "include/instrs.h"

#undef INSTR_TEST
#undef INSTR_TEST_REPEAT

struct InstrTest {
  const char *name;
  int repeat;
  const char *inst;
  void (*test)(int);
};

#define INSTR_TEST(NAME, INST, ...)                                            \
  InstrTest{.name = #NAME, .repeat = 1, .inst = #INST, .test = test_##NAME},
#define INSTR_TEST_REPEAT(NAME, REPEAT, INST, ...)                             \
  InstrTest{                                                                   \
      .name = #NAME, .repeat = REPEAT, .inst = #INST, .test = test_##NAME},

std::vector<InstrTest> tests = {
#include "include/instrs.h"
};

#undef INSTR_TEST

struct InstrInfo {
  std::set<double> latency;
  // cpi
  double throughput;
};

bool instruction_latency_use_perf = false;
int instruction_latency_loop_count = 100000;
void instruction_latency(FILE *fp) {
  bind_to_core();
  if (instruction_latency_use_perf) {
    setup_time_or_cycles();
  }

  std::map<std::string, InstrInfo> info;

  for (auto it : tests) {
    std::string name = it.name;
    if (name == "unit") {
      continue;
    }

    // calibrate unit cycle time continuously
    uint64_t begin = get_time_or_cycles();
    test_unit(instruction_latency_loop_count);
    uint64_t unit_elapsed = get_time_or_cycles() - begin;

    begin = get_time_or_cycles();
    it.test(instruction_latency_loop_count);
    uint64_t elapsed = get_time_or_cycles() - begin;
    double cycles = (double)elapsed / unit_elapsed / it.repeat;

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
      cycles = (double)(long)(cycles * 100 + 0.5) / 100.0;
      printf("%s: latency %.2lf cycles\n", it.name, cycles);
      info[base_name].latency.insert(cycles);
      fflush(stdout);
    }
  }

  fprintf(fp, "name,latency,throughput(cpi),throughput(ipc)\n");
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
    fprintf(fp, "%s,%s,%.2lf,%.2lf\n", pair.first.c_str(), latency.c_str(),
            entry.throughput, 1.0 / entry.throughput);
  }
}
