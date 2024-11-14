// make clangd happy
#ifndef DEFINE_COUNTER
#define DEFINE_COUNTER(...)
#endif

#ifndef DEFINE_COUNTER_RANGE
#define DEFINE_COUNTER_RANGE(...)
#endif

#ifndef DEFINE_COUNTER_SUBTRACT
#define DEFINE_COUNTER_SUBTRACT(...)
#endif

#ifndef DEFINE_COMPUTED_COUNTER_RANGE
#define DEFINE_COMPUTED_COUNTER_RANGE(...)
#endif

#ifdef __APPLE__
// macOS/iOS
DEFINE_COUNTER(cycles, FIXED_CYCLES)
DEFINE_COUNTER(instructions, FIXED_INSTRUCTIONS)
DEFINE_COUNTER(branch_misses, BRANCH_MISPRED_NONSPEC)

#else
// Linux
// select pmu based on icestorm/firestorm
// 0xb: firestorm pmu
#define PERF_TYPE_FIRESTORM 0xb
// 0xa: icestorm pmu
#define PERF_TYPE_ICESTORM 0xa

// 0xa: gracemont pmu
#define PERF_TYPE_GRACEMONT 0xaL

// firestorm/icestorm
// 0x02: CORE_ACTIVE_CYCLE from
// https://github.com/jiegec/apple-pmu/blob/master/a14.md
DEFINE_COUNTER(cycles, firestorm, PERF_TYPE_FIRESTORM, 0x02)
// 0x8c: INST_ALL from
// https://github.com/jiegec/apple-pmu/blob/master/a14.md
DEFINE_COUNTER(instructions, firestorm, PERF_TYPE_FIRESTORM, 0x8c)
// 0xcb: BRANCH_MISPRED_NONSPEC from
// https://github.com/jiegec/apple-pmu/blob/master/a14.md
DEFINE_COUNTER(branch_misses, firestorm, PERF_TYPE_FIRESTORM, 0xcb)

// arm64 general
// ARMV8_PMUV3_PERFCTR_BR_MIS_PRED_RETIRED in
// linux/include/linux/perf/arm_pmuv3.h PERF_COUNT_HW_BRANCH_MISSES was mapped
// to ARMV8_PMUV3_PERFCTR_BR_MIS_PRED, which counts speculative mis-predictions,
// we want retired mis-predictions
DEFINE_COUNTER_RANGE(branch_misses, arm64, PERF_TYPE_RAW, 0x22)

// fallback counters

// cycles
DEFINE_COUNTER_RANGE(cycles, all, PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES)

// instructions retired
DEFINE_COUNTER_RANGE(instructions, all, PERF_TYPE_HARDWARE,
                     PERF_COUNT_HW_INSTRUCTIONS)

// branch mispredictions
DEFINE_COUNTER_RANGE(branch_misses, all, PERF_TYPE_HARDWARE,
                     PERF_COUNT_HW_BRANCH_MISSES)

#endif
