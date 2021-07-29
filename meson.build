project('cpu-micro-benchmarks', 'cpp',
	default_options : ['cpp_std=c++11'])

utils = static_library('utils', 'src/utils.cpp')

memory_latency = executable('memory_latency',
	'src/memory_latency.cpp',
	link_with: utils)

instruction_latency = executable('instruction_latency',
	'src/instruction_latency.cpp',
	link_with: utils)

if host_machine.system() == 'linux'
	instruction_latency_perf = executable('instruction_latency_perf',
		'src/instruction_latency_perf.cpp',
		link_with: utils)
endif