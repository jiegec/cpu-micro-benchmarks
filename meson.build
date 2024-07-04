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

cpu = target_machine.cpu_family()
if cpu == 'x86_64'
	gather_avx2 = executable('gather_avx2',
		'src/gather.cpp',
		cpp_args: ['-DAVX2', '-mavx2'],
		link_with: utils,
		install: true)
	gather_avx512 = executable('gather_avx512',
		'src/gather.cpp',
		cpp_args: ['-DAVX512', '-mavx512f'],
		link_with: utils,
		install: true)
endif



generate_gadget = executable('generate_gadget',
	'src/generate_gadget.cpp',
	link_with: utils)

rob_gadget_asm = custom_target('rob_gadget_asm',
	output: 'rob_gadget.S',
	input: generate_gadget,
	command: [generate_gadget, 'rob', '@OUTPUT@'])

rob_size = executable('rob_size',
	'src/rob_size.cpp',
	rob_gadget_asm,
	link_with: utils,
	install: true)

btb_gadget_asm = custom_target('btb_gadget_asm',
	output: 'btb_gadget.S',
	input: generate_gadget,
	command: [generate_gadget, 'btb', '@OUTPUT@'])

btb_size = executable('btb_size',
	'src/btb_size.cpp',
	btb_gadget_asm,
	link_with: utils,
	install: true)

ras_gadget_asm = custom_target('ras_gadget_asm',
	output: 'ras_gadget.S',
	input: generate_gadget,
	command: [generate_gadget, 'ras', '@OUTPUT@'])

ras_size = executable('ras_size',
	'src/ras_size.cpp',
	ras_gadget_asm,
	link_with: utils,
	install: true)