project('cpu-micro-benchmarks', 'cpp',
	default_options : ['cpp_std=c++11'])

utils = static_library('utils', 'src/utils.cpp')

memory_latency = executable('memory_latency',
	'src/memory_latency.cpp',
	link_with: utils,
	install: true)

instruction_latency = executable('instruction_latency',
	'src/instruction_latency.cpp',
	link_with: utils,
	install: true)

ipc = executable('ipc',
	'src/ipc.cpp',
	link_with: utils,
	install: true)

generate_gadget = executable('generate_gadget',
	'src/generate_gadget.cpp',
	link_with: utils)

gadget_asm = custom_target('gadget_asm',
	output: 'gadget.S',
	input: generate_gadget,
	command: [generate_gadget, '@OUTPUT@'])

rob_size = executable('rob_size',
	'src/rob_size.cpp',
	gadget_asm,
	link_with: utils,
	install: true)
