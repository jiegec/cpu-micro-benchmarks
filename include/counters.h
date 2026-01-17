// declare all raw counters
#include "include/utils.h"
DECLARE_RAW_COUNTER(cycles)
DECLARE_RAW_COUNTER(cycles_active)
DECLARE_RAW_COUNTER(instructions)
DECLARE_RAW_COUNTER(uops)
DECLARE_RAW_COUNTER(mops)

// branches
DECLARE_RAW_COUNTER(branch_instructions)
DECLARE_RAW_COUNTER(branch_misses)
DECLARE_RAW_COUNTER(cond_branch_misses)
DECLARE_RAW_COUNTER(indir_branch_misses)

// caches
DECLARE_RAW_COUNTER(l1i_misses)
DECLARE_RAW_COUNTER(l1d_misses)
DECLARE_RAW_COUNTER(l1d_hwprf)
DECLARE_RAW_COUNTER(l2c_misses)
DECLARE_RAW_COUNTER(l2c_accesses)
DECLARE_RAW_COUNTER(llc_misses)
DECLARE_RAW_COUNTER(llc_accesses)

// itlb
DECLARE_RAW_COUNTER(l1itlb_misses)
DECLARE_RAW_COUNTER(l2itlb_misses)

// dtlb
DECLARE_RAW_COUNTER(l1dtlb_misses)
DECLARE_RAW_COUNTER(l2dtlb_misses)

// declare computed counters
DECLARE_COMPUTED_COUNTER(counter_per_cycle, instructions_per_cycle)