project('cpu-micro-benchmarks', 'cpp',
	default_options : ['cpp_std=c++11'])

# Detect Apple M1
r = run_command('cat', '/sys/devices/system/node/node0/cpu0/of_node/compatible', check: false)
cpp_args = []
if r.stdout().strip().contains('apple,icestorm')
	message('Apple M1 detected')
	if get_option('apple_m1') == 'icestorm'
		message('Configured for Apple M1 Icestorm')
		cpp_args += ['-DAPPLE_M1_ICESTORM', '-DAPPLE_M1']
	else
		message('Configured for Apple M1 Firestorm')
		cpp_args += ['-DAPPLE_M1_FIRESTORM', '-DAPPLE_M1']
	endif
endif

utils = static_library('utils', 'src/utils.cpp', 'src/uarch.cpp', cpp_args: cpp_args)

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

bp_gadget_asm = custom_target('bp_gadget_asm',
	output: 'bp_gadget.S',
	input: generate_gadget,
	command: [generate_gadget, 'bp', '@OUTPUT@'])

bp_size = executable('bp_size',
	'src/bp_size.cpp',
	bp_gadget_asm,
	link_with: utils,
	install: true)

ghr_gadget_asm = custom_target('ghr_gadget_asm',
	output: 'ghr_gadget.S',
	input: generate_gadget,
	command: [generate_gadget, 'ghr', '@OUTPUT@'])

ghr_size = executable('ghr_size',
	'src/ghr_size.cpp',
	ghr_gadget_asm,
	link_with: utils,
	install: true)

cpp = meson.get_compiler('cpp')
cpu = target_machine.cpu_family()

if cpu == 'x86_64'
	ghr2_gadget_asm = custom_target('ghr2_gadget_asm',
		output: 'ghr2_gadget.S',
		input: generate_gadget,
		command: [generate_gadget, 'ghr2', '@OUTPUT@'])

	ghr2_gadget_o = custom_target('ghr2_gadget_o',
		output: 'ghr2_gadget.o',
		input: ghr2_gadget_asm,
		command: ['nasm', '-felf64', '@INPUT@', '-o', '@OUTPUT@'])

	ghr2_size = executable('ghr2_size',
		'src/ghr2_size.cpp',
		ghr2_gadget_o,
		link_with: utils,
		install: true)
elif cpu == 'aarch64'
	ghr2_gadget_asm = custom_target('ghr2_gadget_asm',
		output: 'ghr2_gadget.S',
		input: generate_gadget,
		command: [generate_gadget, 'ghr2', '@OUTPUT@'])

	ghr2_size = executable('ghr2_size',
		'src/ghr2_size.cpp',
		ghr2_gadget_asm,
		link_with: utils,
		install: true)
endif