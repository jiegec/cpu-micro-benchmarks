// declare all raw counters
#include "include/utils.h"
DECLARE_RAW_COUNTER(cycles)
DECLARE_RAW_COUNTER(instructions)
DECLARE_RAW_COUNTER(branch_misses)
DECLARE_RAW_COUNTER(cond_branch_misses)
DECLARE_RAW_COUNTER(llc_misses)
DECLARE_RAW_COUNTER(llc_loads)

// declare computed counters
DECLARE_COMPUTED_COUNTER(counter_per_cycle, instructions_per_cycle)