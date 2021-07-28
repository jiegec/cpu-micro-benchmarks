cc_binary(
    name = "memory_latency",
    srcs = ["src/memory_latency.cpp"],
    deps = [":utils"],
)

cc_binary(
    name = "instruction_latency",
    srcs = [
        "include/instrs.h",
        "src/instruction_latency.cpp",
    ],
    deps = [":utils"],
)

cc_binary(
    name = "instruction_latency_perf",
    srcs = [
        "include/instrs.h",
        "src/instruction_latency_perf.cpp",
    ],
    deps = [":utils"],
)

cc_library(
    name = "utils",
    srcs = ["src/utils.cpp"],
    hdrs = ["include/utils.h"],
)
