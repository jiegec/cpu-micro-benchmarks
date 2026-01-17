project('cpu-micro-benchmarks', 'cpp',
	default_options : ['cpp_std=c++11'])
cpp_args = []
link_args = []
cpu = target_machine.cpu_family()

if get_option('ios')
	message('Configured for iOS')
	cpp_args += ['-DIOS', '-std=c++11', '-DHOST_AARCH64', '-march=armv8.4-a']
elif get_option('android')
	cpp_args += ['-DANDROID', '-DHOST_AARCH64', '-DNO_FJCVTZS']
	link_args += ['-static-libstdc++']
elif get_option('gem5')
	cpp_args += ['-DGEM5']
elif get_option('linux-cross') == 'aarch64'
	cpp_args += ['-DHOST_AARCH64', '-DNO_FJCVTZS']
else
	# Detect cpu models
	r = run_command('sh', '-c', 'g++ -I. --std=c++11 src/detect_uarch.cpp src/uarch.cpp -o $MESON_BUILD_ROOT/detect_uarch && $MESON_BUILD_ROOT/detect_uarch', check: true)
	foreach line : r.stderr().strip().split('\n')
		message(line)
	endforeach
	foreach line : r.stdout().strip().split('\n')
		cpp_args += [line]
	endforeach
	message('Got CXXFLAGS:', cpp_args)
endif


message('Final CXXFLAGS:', cpp_args)
message('Final LDFLAGS:', link_args)

utils = static_library('utils', 'src/utils.cpp', 'src/uarch.cpp', cpp_args: cpp_args)
utils_native = static_library('utils_native', 'src/utils.cpp', 'src/uarch.cpp', cpp_args: cpp_args, native : true)

simple_progs = [
	'dtlb_size',
	'memory_latency',
]

foreach p : simple_progs
	executable(p,
		'src/' + p + '.cpp',
		cpp_args: cpp_args,
		link_with: utils)
endforeach

libs = []

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

	gather_serialize_avx2 = executable('gather_serialize_avx2',
		'src/gather_serialize.cpp',
		cpp_args: ['-DAVX2', '-mavx2'],
		link_with: utils,
		install: true)

	gather_serialize_avx512 = executable('gather_serialize_avx512',
		'src/gather_serialize.cpp',
		cpp_args: ['-DAVX512', '-mavx512f'],
		link_with: utils,
		install: true)
elif cpu == 'aarch64'
	gather = executable('gather',
		'src/gather.cpp',
		cpp_args: ['-DSVE', '-march=armv8-a+sve'],
		link_with: utils,
		install: true)

	gather_serialize = executable('gather_serialize',
		'src/gather_serialize.cpp',
		cpp_args: ['-DSVE', '-march=armv8-a+sve'],
		link_with: utils,
		install: true)
endif

cpp = meson.get_compiler('cpp')
cpu = target_machine.cpu_family()

# [gadget name, nasm if available, build as library, has generator]
progs = [
	['bp_size', false, false, true],
	['btb_size_basic', false, true, true],
	['elimination', false, true, true],
	['fetch_bandwidth', false, true, true],
	['find_branch_misses_pmu', false, false, true],
	['fp_peak', false, false, true],
	['ghr_size', false, true, true],
	['if_width', false, true, true],
	['instruction_latency', false, true, false],
	['itlb_size', false, true, true],
	['phr_branch_bits_location', true, false, true],
	['phr_size', false, true, true],
	['phr_target_bits_location', true, false, true],
	['phr_branch_target_xor', true, false, true],
	['pht_associativity', true, false, true],
	['pht_index_bits_xor', false, false, true],
	['pht_index_bits_xor_phr', false, false, true],
	['pht_index_tag_bits', true, false, true],
	['pht_tag_bits_xor', false, false, true],
	['pht_tag_bits_xor_phr', false, false, true],
	['ras_size', false, true, true],
	['register_file_size', false, true, true],
	['rob_size', false, false, true],
	['sched_size', false, true, true],
]

foreach p : progs
	# generator?
	if p[3]
		gadget_gen = executable(p[0] + '_gadget_gen',
			'src/' + p[0] + '_gen.cpp',
			cpp_args: cpp_args,
			link_with: utils_native,
			native : true)

		gadget_asm = custom_target(p[0] + '_gadget_asm',
			output: p[0] + '_gadget.S',
			input: gadget_gen,
			command: [gadget_gen, '@OUTPUT@'])
	
		# nasm?
		if cpu == 'x86_64' and p[1]
			gadget_o = custom_target(p[0] + '_gadget_o',
				output: p[0] + '_gadget.o',
				input: gadget_asm,
				command: ['nasm', '--limit-rep', '10000000', '-felf64', '@INPUT@', '-o', '@OUTPUT@', '-O0'])

			gadget_src = gadget_o
		else
			gadget_src = gadget_asm
		endif
		
		# library?
		if p[2]
			gadget_lib = static_library(p[0] + '_lib',
				'src/' + p[0] + '_lib.cpp',
				gadget_src,
				cpp_args: cpp_args,
				link_with: utils,
				install: true)
			libs += gadget_lib.extract_all_objects(recursive: true)

			gadget = executable(p[0],
				'src/' + p[0] + '.cpp',
				cpp_args: cpp_args,
				link_with: gadget_lib,
				link_args: link_args,
				install: true)
		else
			gadget = executable(p[0],
				'src/' + p[0] + '.cpp',
				gadget_src,
				cpp_args: cpp_args,
				link_with: utils,
				link_args: link_args,
				install: true)
		endif
	else
		# library?
		if p[2]
			gadget_lib = static_library(p[0] + '_lib',
				'src/' + p[0] + '_lib.cpp',
				cpp_args: cpp_args,
				link_with: utils,
				install: true)
			libs += gadget_lib.extract_all_objects(recursive: true)
			
			gadget = executable(p[0],
				'src/' + p[0] + '.cpp',
				cpp_args: cpp_args,
				link_with: gadget_lib,
				link_args: link_args,
				install: true)
		else
			gadget = executable(p[0],
				'src/' + p[0] + '.cpp',
				gadget_asm,
				cpp_args: cpp_args,
				link_with: utils,
				link_args: link_args,
				install: true)
		endif
	endif
endforeach

# build whole library for ios
if get_option('ios')
	message('Building libmicrobenchmark')
	microbenchmark_lib = static_library('microbenchmark',
		objects: libs,
		link_with: utils,
		install: true)
endif

if target_machine.system() == 'linux'
	# required linux affinity api
	coherency_latency = executable('coherency_latency',
		'src/coherency_latency.cpp',
		link_args: ['-pthread'],
		install: true)
endif
