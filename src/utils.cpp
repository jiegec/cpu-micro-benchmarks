#include "include/utils.h"
#include "include/uarch.h"
#include <assert.h>
#include <cstdint>
#include <fcntl.h>
#include <map>
#include <memory.h>
#include <random>
#include <regex>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#ifdef __x86_64__
#include <x86intrin.h>
#endif

#if defined(__APPLE__) && !defined(IOS)
#include <dlfcn.h>
#include <sys/kdebug.h>
#include <sys/sysctl.h>
#endif

#ifdef __linux__
#include <linux/perf_event.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#endif

#ifdef __x86_64__
#include <x86intrin.h>
#endif

// https://clang.llvm.org/docs/LanguageExtensions.html#has-builtin
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

std::map<const char *, size_t> get_cache_sizes() {
  std::map<const char *, size_t> result;
#ifdef __linux__
  int keys[] = {_SC_LEVEL1_ICACHE_SIZE, _SC_LEVEL1_DCACHE_SIZE,
                _SC_LEVEL2_CACHE_SIZE, _SC_LEVEL3_CACHE_SIZE};
  const char *names[] = {
      "L1i",
      "L1d",
      "L2",
      "L3",
  };
  for (int i = 0; i < 3; i++) {
    long size = sysconf(keys[i]);
    if (size != -1) {
      result[names[i]] = size;
    }
  }
#elif defined(__APPLE__) && !defined(IOS)
  const char *keys[] = {"hw.l1dcachesize", "hw.l1icachesize", "hw.l2cachesize"};
  const char *names[] = {
      "L1d",
      "L1i",
      "L2",
  };
  for (int i = 0; i < 3; i++) {
    size_t size = 0;
    size_t len = sizeof(size);
    int ret = sysctlbyname(keys[i], &size, &len, nullptr, 0);
    if (ret != -1) {
      result[names[i]] = size;
    }
  }
#endif
  return result;
}

uint64_t get_time() {
  struct timeval tv = {};
  gettimeofday(&tv, nullptr);
  return (uint64_t)tv.tv_sec * 1000000000 + (uint64_t)tv.tv_usec * 1000;
}

char **generate_random_pointer_chasing(size_t size, size_t granularity) {
  if (granularity == (size_t)-1) {
    // use page size as granularity
    granularity = getpagesize();
  }

  if (size < granularity) {
    return NULL;
  }

  // number of pointers within each `granularity` bytes
  int pointer_count = granularity / sizeof(char *);
  int count = size / sizeof(char *);
  // every `granularity` bytes has one pointer
  int index_count = size / granularity;
  char **buffer = new char *[count];
  int *index = new int[index_count];

  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());

  // init index and shuffle
  for (int i = 0; i < index_count; i++) {
    index[i] = i;
  }
  for (int i = 1; i < index_count; i++) {
    std::uniform_int_distribution<int> distr(0, i - 1);
    int j = distr(generator);
    int temp = index[i];
    index[i] = index[j];
    index[j] = temp;
  }

  // init circular list
  for (int i = 0; i < index_count - 1; i++) {
    buffer[index[i] * pointer_count] =
        (char *)&buffer[index[i + 1] * pointer_count];
  }
  buffer[index[index_count - 1] * pointer_count] =
      (char *)&buffer[index[0] * pointer_count];

  delete[] index;

  return buffer;
}

#ifdef __linux__
uint64_t raw_perf_counter::read() const {
  if (page->cap_user_rdpmc) {
    return read_userspace();
  } else {
    return read_syscall();
  }
}

uint64_t raw_perf_counter::read_syscall() const {
  // read counter using syscall
  uint64_t counter;
  int res = ::read(fd, &counter, sizeof(counter));
  assert(res == sizeof(counter));
  return counter;
}

// mimic READ_ONCE from linux kernel
#define READ_ONCE(x) (*(const volatile decltype(x) *)&(x))

// copied from
// https://github.com/torvalds/linux/blob/master/tools/lib/perf/mmap.c
#ifdef __x86_64__
static inline uint64_t read_perf_counter(unsigned int counter) {
  return _rdpmc(counter);
}

#elif defined(__aarch64__)
#define __stringify(a) #a
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define read_sysreg(r)                                                         \
  ({                                                                           \
    uint64_t __val;                                                            \
    asm volatile("mrs %0, " __stringify(r) : "=r"(__val));                     \
    __val;                                                                     \
  })

static uint64_t read_pmccntr(void) { return read_sysreg(pmccntr_el0); }

#define PMEVCNTR_READ(idx)                                                     \
  static uint64_t read_pmevcntr_##idx(void) {                                  \
    return read_sysreg(pmevcntr##idx##_el0);                                   \
  }

PMEVCNTR_READ(0);
PMEVCNTR_READ(1);
PMEVCNTR_READ(2);
PMEVCNTR_READ(3);
PMEVCNTR_READ(4);
PMEVCNTR_READ(5);
PMEVCNTR_READ(6);
PMEVCNTR_READ(7);
PMEVCNTR_READ(8);
PMEVCNTR_READ(9);
PMEVCNTR_READ(10);
PMEVCNTR_READ(11);
PMEVCNTR_READ(12);
PMEVCNTR_READ(13);
PMEVCNTR_READ(14);
PMEVCNTR_READ(15);
PMEVCNTR_READ(16);
PMEVCNTR_READ(17);
PMEVCNTR_READ(18);
PMEVCNTR_READ(19);
PMEVCNTR_READ(20);
PMEVCNTR_READ(21);
PMEVCNTR_READ(22);
PMEVCNTR_READ(23);
PMEVCNTR_READ(24);
PMEVCNTR_READ(25);
PMEVCNTR_READ(26);
PMEVCNTR_READ(27);
PMEVCNTR_READ(28);
PMEVCNTR_READ(29);
PMEVCNTR_READ(30);

/*
 * Read a value direct from PMEVCNTR<idx>
 */
static inline uint64_t read_perf_counter(unsigned int counter) {
  static uint64_t (*const read_f[])(void) = {
      read_pmevcntr_0,  read_pmevcntr_1,  read_pmevcntr_2,  read_pmevcntr_3,
      read_pmevcntr_4,  read_pmevcntr_5,  read_pmevcntr_6,  read_pmevcntr_7,
      read_pmevcntr_8,  read_pmevcntr_9,  read_pmevcntr_10, read_pmevcntr_11,
      read_pmevcntr_13, read_pmevcntr_12, read_pmevcntr_14, read_pmevcntr_15,
      read_pmevcntr_16, read_pmevcntr_17, read_pmevcntr_18, read_pmevcntr_19,
      read_pmevcntr_20, read_pmevcntr_21, read_pmevcntr_22, read_pmevcntr_23,
      read_pmevcntr_24, read_pmevcntr_25, read_pmevcntr_26, read_pmevcntr_27,
      read_pmevcntr_28, read_pmevcntr_29, read_pmevcntr_30, read_pmccntr};

  if (counter < ARRAY_SIZE(read_f))
    return (read_f[counter])();

  return 0;
}
#else
static inline uint64_t read_perf_counter(unsigned int counter) { return 0; }
#endif

uint64_t raw_perf_counter::read_userspace() const {
  // read counter in userspace
  // ref: man page of perf_event_open
  // https://github.com/torvalds/linux/blob/master/tools/lib/perf/mmap.c
  uint32_t seq, idx, width;
  uint64_t count;

  do {
    seq = READ_ONCE(page->lock);
    asm volatile("" : : : "memory");

    idx = READ_ONCE(page->index);
    count = READ_ONCE(page->offset);

    if (page->cap_user_rdpmc && idx) {
      int64_t pmc = read_perf_counter(idx - 1);
      width = READ_ONCE(page->pmc_width);

      // sign extension
      pmc <<= 64 - width;
      pmc >>= 64 - width;
      count += pmc;
    }

    asm volatile("" : : : "memory");
  } while (READ_ONCE(page->lock) != seq);

  return count;
}

raw_perf_counter::raw_perf_counter() {
  fd = -1;
  page = (struct perf_event_mmap_page *)MAP_FAILED;
  id = 0;
}

void raw_perf_counter::reset() const { ioctl(fd, PERF_EVENT_IOC_RESET, 0); }

raw_perf_counter setup_perf_common_failable(uint32_t type, uint64_t config,
                                            int group_fd, int read_format,
                                            int flags, uint64_t sample_type) {
  raw_perf_counter res;
  // call perf_event_open
  struct perf_event_attr *attr =
      (struct perf_event_attr *)malloc(sizeof(struct perf_event_attr));
  memset(attr, 0, sizeof(struct perf_event_attr));
  attr->type = type;
  attr->size = sizeof(struct perf_event_attr);
  attr->config = config;
  attr->disabled = 0;
  attr->inherit = 0; // 0 is required for non-zero sample_type & mmap
  attr->exclude_kernel = 1;
  attr->exclude_guest = 1;
  attr->read_format = read_format;
  attr->sample_type = sample_type;
  int fd = syscall(SYS_perf_event_open, attr, 0, -1, group_fd, flags);
  free(attr);

  res.fd = fd;
  if (fd >= 0) {
    // mmap for userspace access
    res.page = (struct perf_event_mmap_page *)mmap(0, getpagesize(), PROT_READ,
                                                   MAP_SHARED, res.fd, 0);
    // id for grouping
    ioctl(res.fd, PERF_EVENT_IOC_ID, &res.id);
  }
  return res;
}

raw_perf_counter setup_perf_common_failable(uint32_t type, uint64_t config) {
  return setup_perf_common_failable(type, config, -1, 0, 0, 0);
}

raw_perf_counter setup_perf_common(uint32_t type, uint64_t config, int group_fd,
                                   int read_format, int flags,
                                   uint64_t sample_type) {
  raw_perf_counter res = setup_perf_common_failable(
      type, config, group_fd, read_format, flags, sample_type);
  if (res.fd < 0 || res.page == MAP_FAILED) {
    perror("perf_event_open");
    fprintf(stderr, "failed to setup perf event, try: sudo sysctl "
                    "kernel.perf_event_paranoid=2\n");
    exit(1);
  }
  return res;
}

raw_perf_counter setup_perf_common(uint32_t type, uint64_t config) {
  return setup_perf_common(type, config, -1, 0, 0, 0);
}

struct counter_mapping {
  const char *name;
  // match uarch in range [uarch_begin, uarch_end]
  enum uarch uarch_begin;
  enum uarch uarch_end;
  uint32_t type;
  uint64_t config;

  // for computed counter
  const char *source_counters;
  void *compute_fn;
};

static uint64_t compute_subtract(const std::vector<uint64_t> counters) {
  assert(counters.size() == 2);
  return counters[0] - counters[1];
}

static counter_per_cycle
compute_counter_per_cycle(const std::vector<uint64_t> counters) {
  assert(counters.size() == 2);
  counter_per_cycle res;
  res.counter = counters[0];
  res.cycles = counters[1];
  return res;
}

// collect counter mappings
std::vector<counter_mapping> counter_mappings = {
#define DEFINE_COUNTER(_name, _uarch, _type, _config)                          \
  counter_mapping{#_name, _uarch, _uarch, _type, _config, NULL, NULL},
#define DEFINE_COUNTER_RANGE(_name, _uarch, _type, _config)                    \
  counter_mapping{#_name,  _uarch##_begin, _uarch##_end, _type,                \
                  _config, NULL,           NULL},
#define DEFINE_COMPUTED_COUNTER(_name, _ret_type, _uarch, _fn, ...)            \
  counter_mapping{#_name, _uarch, _uarch, 0, 0, #__VA_ARGS__, (void *)_fn},
#define DEFINE_COMPUTED_COUNTER_RANGE(_name, _ret_type, _uarch, _fn, ...)      \
  counter_mapping{#_name, _uarch##_begin, _uarch##_end, 0,                     \
                  0,      #__VA_ARGS__,   (void *)_fn},
#include "include/counters_mapping.h"
#undef DEFINE_COUNTER
#undef DEFINE_COMPUTED_COUNTER_RANGE
};

struct counter_mapping find_mapping(const char *name) {
  enum uarch current = get_uarch();
  printf("Looking for PMU counter %s\n", name);
  for (auto mapping : counter_mappings) {
    if (strcmp(mapping.name, name) == 0 && (current >= mapping.uarch_begin) &&
        (current <= mapping.uarch_end)) {
      if (mapping.source_counters) {
        printf("Found perf counter for %s: computed from %s\n", name,
               mapping.source_counters);
      } else {
        printf("Found perf counter for %s: type=0x%x config=0x%lx\n", name,
               mapping.type, mapping.config);
      }
      return mapping;
    }
  }
  assert(false);
}

std::vector<std::string> split_counters(const std::string &counters) {
  // ref: https://stackoverflow.com/a/10058756/2148614
  std::vector<std::string> tokens;
  std::regex re("[a-z_]+");

  std::sregex_token_iterator begin(counters.begin(), counters.end(), re), end;

  std::copy(begin, end, std::back_inserter(tokens));
  return tokens;
}

#define DECLARE_RAW_COUNTER(name)                                              \
  static raw_perf_counter perf_counter_##name;                                 \
  static raw_perf_counter perf_counter_##name##_2;                             \
  uint64_t perf_read_##name() {                                                \
    if (perf_counter_##name##_2.fd == -1)                                      \
      return perf_counter_##name.read();                                       \
    else                                                                       \
      return perf_counter_##name.read() - perf_counter_##name##_2.read();      \
  }                                                                            \
  void setup_perf_##name() {                                                   \
    fprintf(stderr, "Recording PMU counter for %s\n", #name);                  \
    counter_mapping mapping = find_mapping(#name);                             \
    perf_counter_##name = setup_perf_common(mapping.type, mapping.config);     \
  }

#define DECLARE_COMPUTED_COUNTER(_type, name)                                  \
  static std::vector<raw_perf_counter> perf_counter_##name;                    \
  static std::vector<uint64_t> perf_value_##name;                              \
  static void *perf_compute_fn_##name = NULL;                                  \
  void perf_begin_##name() {                                                   \
    perf_value_##name.clear();                                                 \
    for (auto counter : perf_counter_##name) {                                 \
      perf_value_##name.push_back(counter.read());                             \
    }                                                                          \
  }                                                                            \
  _type perf_end_##name() {                                                    \
    std::vector<uint64_t> elapsed;                                             \
    for (size_t i = 0; i < perf_counter_##name.size(); i++) {                  \
      elapsed.push_back(perf_counter_##name[i].read() - perf_value_##name[i]); \
    }                                                                          \
    return ((_type(*)(const std::vector<uint64_t> &))perf_compute_fn_##name)(  \
        elapsed);                                                              \
  }                                                                            \
  void setup_perf_##name() {                                                   \
    fprintf(stderr, "Recording PMU counter for %s\n", #name);                  \
    counter_mapping mapping = find_mapping(#name);                             \
    assert(mapping.compute_fn);                                                \
    perf_compute_fn_##name = mapping.compute_fn;                               \
    for (auto counter : split_counters(mapping.source_counters)) {             \
      counter_mapping source_mapping = find_mapping(counter.c_str());          \
      perf_counter_##name.push_back(                                           \
          setup_perf_common(source_mapping.type, source_mapping.config));      \
    }                                                                          \
  }

#include "include/counters.h"
#undef DECLARE_RAW_COUNTER

#elif defined(__APPLE__) && !defined(IOS)
// macOS
// https://gist.github.com/ibireme/173517c208c7dc333ba962c1f0d67d12

#define KPC_MAX_COUNTERS 32
#define KPC_CLASS_CONFIGURABLE_MASK (1u << 1)
typedef struct kpep_db {
  const char *name;           ///< Database name, such as "haswell".
  const char *cpu_id;         ///< Plist name, such as "cpu_7_8_10b282dc".
  const char *marketing_name; ///< Marketing name, such as "Intel Haswell".
                              // more fields omitted
} kpep_db;
typedef struct kpep_config {
} kpep_config;
typedef struct kpep_event {
} kpep_event;
typedef uint64_t kpc_config_t;

static int (*kpc_force_all_ctrs_get)(int *val_out);
static int (*kpc_force_all_ctrs_set)(int val);
static int (*kpc_set_counting)(uint32_t classes);
static int (*kpc_set_thread_counting)(uint32_t classes);
static int (*kpc_get_thread_counters)(uint32_t tid, uint32_t buf_count,
                                      uint64_t *buf);
static int (*kpc_set_config)(uint32_t classes, kpc_config_t *config);

static int (*kpep_db_create)(const char *name, kpep_db **db_ptr);
static int (*kpep_config_create)(kpep_db *db, kpep_config **cfg_ptr);
static int (*kpep_config_force_counters)(kpep_config *cfg);
static int (*kpep_db_event)(kpep_db *db, const char *name, kpep_event **ev_ptr);
static int (*kpep_config_add_event)(kpep_config *cfg, kpep_event **ev_ptr,
                                    uint32_t flag, uint32_t *err);
static int (*kpep_config_kpc_classes)(kpep_config *cfg, uint32_t *classes_ptr);
static int (*kpep_config_kpc_count)(kpep_config *cfg, size_t *count_ptr);
static int (*kpep_config_kpc_map)(kpep_config *cfg, size_t *buf,
                                  size_t buf_size);
static int (*kpep_config_kpc)(kpep_config *cfg, kpc_config_t *buf,
                              size_t buf_size);

static void *lib_kperf = NULL;
static void *lib_kperfdata = NULL;
static kpep_db *db = NULL;
static kpep_config *config = NULL;
static bool started = false;
static size_t counter_map[KPC_MAX_COUNTERS] = {};
static std::map<std::string, size_t> event_map;
void setup_kperf() {
  // load shared libraries
  lib_kperf = dlopen("/System/Library/PrivateFrameworks/kperf.framework/kperf",
                     RTLD_LAZY);
  assert(lib_kperf);
  lib_kperfdata =
      dlopen("/System/Library/PrivateFrameworks/kperfdata.framework/kperfdata",
             RTLD_LAZY);
  assert(lib_kperfdata);

  // see if we can access kpc
  kpc_force_all_ctrs_get =
      (int (*)(int *))dlsym(lib_kperf, "kpc_force_all_ctrs_get");
  assert(kpc_force_all_ctrs_get);
  int force_ctrs = 0;
  if (kpc_force_all_ctrs_get(&force_ctrs) != 0) {
    fprintf(stderr, "Root permission is required for pmu on macOS\n");
    exit(1);
  }

  // create db
  kpep_db_create =
      (int (*)(const char *, kpep_db **))dlsym(lib_kperfdata, "kpep_db_create");
  assert(kpep_db_create);
  assert(kpep_db_create(NULL, &db) == 0);
  fprintf(stderr, "Loaded PMU database: %s (%s, %s)\n", db->name,
          db->marketing_name, db->cpu_id);

  // create config
  kpep_config_create = (int (*)(kpep_db *, kpep_config **))dlsym(
      lib_kperfdata, "kpep_config_create");
  assert(kpep_config_create);
  assert(kpep_config_create(db, &config) == 0);

  // force counters
  kpep_config_force_counters = (int (*)(kpep_config *))dlsym(
      lib_kperfdata, "kpep_config_force_counters");
  assert(kpep_config_force_counters);
  assert(kpep_config_force_counters(config) == 0);

  // dlsym the rest functions
  kpep_db_event = (int (*)(kpep_db *, const char *, kpep_event **))dlsym(
      lib_kperfdata, "kpep_db_event");
  assert(kpep_db_event);

  kpep_config_add_event =
      (int (*)(kpep_config *, kpep_event **, uint32_t, uint32_t *))dlsym(
          lib_kperfdata, "kpep_config_add_event");
  assert(kpep_config_add_event);

  kpep_config_kpc_classes = (int (*)(kpep_config *, uint32_t *))dlsym(
      lib_kperfdata, "kpep_config_kpc_classes");
  assert(kpep_config_kpc_classes);

  kpep_config_kpc_count = (int (*)(kpep_config *, size_t *))dlsym(
      lib_kperfdata, "kpep_config_kpc_count");
  assert(kpep_config_kpc_count);

  kpep_config_kpc_map = (int (*)(kpep_config *, size_t *, size_t))dlsym(
      lib_kperfdata, "kpep_config_kpc_map");
  assert(kpep_config_kpc_map);

  kpep_config_kpc = (int (*)(kpep_config *, kpc_config_t *, size_t))dlsym(
      lib_kperfdata, "kpep_config_kpc");
  assert(kpep_config_kpc);

  kpc_force_all_ctrs_set =
      (int (*)(int))dlsym(lib_kperf, "kpc_force_all_ctrs_set");
  assert(kpc_force_all_ctrs_set);

  kpc_set_counting = (int (*)(uint32_t))dlsym(lib_kperf, "kpc_set_counting");
  assert(kpc_set_counting);

  kpc_set_thread_counting =
      (int (*)(uint32_t))dlsym(lib_kperf, "kpc_set_thread_counting");
  assert(kpc_set_thread_counting);

  kpc_get_thread_counters = (int (*)(uint32_t, uint32_t, uint64_t *))dlsym(
      lib_kperf, "kpc_get_thread_counters");
  assert(kpc_get_thread_counters);

  kpc_set_config =
      (int (*)(uint32_t, kpc_config_t *))dlsym(lib_kperf, "kpc_set_config");
  assert(kpc_set_config);
}

void setup_perf_common(const char *name) {
  if (!lib_kperf) {
    setup_kperf();
  }

  // find event in db
  kpep_event *event = NULL;
  assert(kpep_db_event(db, name, &event) == 0);
  fprintf(stderr, "Found event for counter %s\n", name);

  // add event to db
  assert(kpep_config_add_event(config, &event, 0, NULL) == 0);

  // record index of event
  size_t index = event_map.size();
  event_map[name] = index;
}

void start_counters() {
  // enable pmu counters
  uint32_t classes = 0;
  size_t reg_count = 0;
  kpc_config_t regs[KPC_MAX_COUNTERS] = {};

  // setup counters
  assert(kpep_config_kpc_classes(config, &classes) == 0);
  assert(kpep_config_kpc_count(config, &reg_count) == 0);
  assert(kpep_config_kpc_map(config, counter_map, sizeof(counter_map)) == 0);
  assert(kpep_config_kpc(config, regs, sizeof(regs)) == 0);

  // enable counters
  assert(kpc_force_all_ctrs_set(1) == 0);
  if ((classes & KPC_CLASS_CONFIGURABLE_MASK) && reg_count) {
    assert(kpc_set_config(classes, regs) == 0);
  }
  assert(kpc_set_counting(classes) == 0);
  assert(kpc_set_thread_counting(classes) == 0);
}

uint64_t perf_read_common(const char *name) {
  if (!started) {
    // initialize counters after all events are found
    started = true;
    start_counters();
  }

  uint64_t counters[KPC_MAX_COUNTERS] = {};
  assert(kpc_get_thread_counters(0, KPC_MAX_COUNTERS, counters) == 0);
  // find corresponding counter from counter_map
  int index = counter_map[event_map[name]];
  return counters[index];
}

counter_per_cycle perf_read_common_per_cycle(const char *name) {
  if (!started) {
    // initialize counters after all events are found
    started = true;
    start_counters();
  }

  uint64_t counters[KPC_MAX_COUNTERS] = {};
  assert(kpc_get_thread_counters(0, KPC_MAX_COUNTERS, counters) == 0);

  counter_per_cycle res;

  // find corresponding counter from counter_map
  int index = counter_map[event_map[name]];
  res.counter = counters[index];

  index = counter_map[event_map[std::string("FIXED_CYCLES")]];
  res.cycles = counters[index];

  return res;
}

#define DEFINE_COUNTER(name, event)                                            \
  static counter_per_cycle perf_counter_per_cycle_##name;                      \
  uint64_t perf_read_##name() { return perf_read_common(#event); }             \
  void setup_perf_##name() {                                                   \
    fprintf(stderr, "Recording PMU counter for %s\n", #name);                  \
    setup_perf_common(#event);                                                 \
  }                                                                            \
  void setup_perf_##name##_per_cycle() {                                       \
    fprintf(stderr, "Recording PMU counter for %s per cycle\n", #name);        \
    setup_perf_common("FIXED_CYCLES");                                         \
    setup_perf_common(#event);                                                 \
  }                                                                            \
  void perf_begin_##name##_per_cycle() {                                       \
    perf_counter_per_cycle_##name = perf_read_common_per_cycle(#event);        \
  }                                                                            \
  counter_per_cycle perf_end_##name##_per_cycle() {                            \
    counter_per_cycle res = perf_read_common_per_cycle(#event);                \
    res.counter -= perf_counter_per_cycle_##name.counter;                      \
    res.cycles -= perf_counter_per_cycle_##name.cycles;                        \
    return res;                                                                \
  }

// TODO
void setup_perf_top_down() {}
void perf_begin_top_down() {}
top_down perf_end_top_down() { return top_down{}; }

#include "include/counters_mapping.h"
#undef DEFINE_COUNTER
#elif defined(__APPLE__) && defined(IOS)
// ios

// Adapted from
// https://github.com/junjie1475/iOS-microbench/blob/main/iOS-microbench/main.c

struct proc_threadcounts_data {
  uint64_t ptcd_instructions;
  uint64_t ptcd_cycles;
  uint64_t ptcd_user_time_mach;
  uint64_t ptcd_system_time_mach;
  uint64_t ptcd_energy_nj;
};

struct proc_threadcounts {
  uint16_t ptc_len;
  uint16_t ptc_reserved0;
  uint32_t ptc_reserved1;
  struct proc_threadcounts_data ptc_counts[];
};

// https://github.com/apple-oss-distributions/xnu/blob/aca3beaa3dfbd42498b42c5e5ce20a938e6554e5/bsd/sys/proc_info.h#L927
#define PROC_PIDTHREADCOUNTS 34
#define PROC_PIDTHREADCOUNTS_SIZE (sizeof(struct proc_threadcounts))
extern "C" int proc_pidinfo(int pid, int flavor, uint64_t arg, void *buffer,
                            int buffersize);

// only support cycles and instructions

static uint64_t tid;
static int countsize;
static pid_t pid;
static proc_threadcounts *rbuf = NULL;

void setup_perf_common() {
  pid = getpid();
  printf("Got pid %d\n", pid);
  // 2: p and e, two perf levels
  countsize = sizeof(struct proc_threadcounts) +
              2 * sizeof(struct proc_threadcounts_data);
  rbuf = (struct proc_threadcounts *)malloc(countsize);
  memset(rbuf, 0, countsize);
  pthread_threadid_np(pthread_self(), &tid);
  printf("Got tid %d\n", tid);
}

uint64_t perf_read_cycles() {
  proc_pidinfo(pid, PROC_PIDTHREADCOUNTS, tid, rbuf, countsize);
  // read all cores
  return rbuf->ptc_counts[0].ptcd_cycles + rbuf->ptc_counts[1].ptcd_cycles;
}

uint64_t perf_read_instructions() {
  proc_pidinfo(pid, PROC_PIDTHREADCOUNTS, tid, rbuf, countsize);
  // read all cores
  return rbuf->ptc_counts[0].ptcd_instructions +
         rbuf->ptc_counts[1].ptcd_instructions;
}

void setup_perf_cycles() { setup_perf_common(); }

void setup_perf_instructions() { setup_perf_common(); }

// provide dummy impl

#define DEFINE_COUNTER(name, event)                                            \
  uint64_t perf_read_##name() { return 0; }                                    \
  void setup_perf_##name() {}
#include "include/counters_mapping.h"
#undef DEFINE_COUNTER

#endif

void setup_time_or_cycles() { setup_perf_cycles(); }

uint64_t get_time_or_cycles() {
#ifdef __linux__
  if (perf_counter_cycles.fd >= 0) {
#elif defined(__APPLE__) && defined(IOS)
  // perf initialized
  if (rbuf) {
#elif defined(__APPLE__) && !defined(IOS)
  // perf initialized
  if (lib_kperf != NULL) {
#endif
    // cycle
    return perf_read_cycles();
  } else {
    // time
    return get_time();
  }
}

// bind to core
void bind_to_core() {
#ifdef __linux__
  cpu_set_t set;
  CPU_ZERO(&set);
  int core = get_bind_core();

  CPU_SET(core, &set);
  int res = sched_setaffinity(0, sizeof(set), &set);
  if (res == 0) {
    fprintf(stderr, "Pinned to cpu %d\n", core);
  }
#elif defined(__APPLE__) && !defined(IOS)
  int core = get_bind_core();
  // we cannot bind to specific core on macOS:
  // core == 0 means e-core
  // core != 0 means p-core
  if (core == 0) {
    // e core
    fprintf(stderr, "Bind to E core on macOS\n");
    pthread_set_qos_class_self_np(QOS_CLASS_BACKGROUND, 0);
  } else {
    // p core
    fprintf(stderr, "Bind to P core on macOS\n");
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
  }
#elif defined(IOS)
  // TODO: make it configurable
  // it is also not very reliable
  // p core
  fprintf(stderr, "Bind to P core on iOS\n");
  pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif
}

bool nasm = false;

void define_gadgets_array(FILE *fp, const char *name) {
  if (nasm) {
    fprintf(fp, "section .data\n");
    fprintf(fp, "align 8\n");
  } else {
    fprintf(fp, ".data\n");
    fprintf(fp, ".balign 8\n");
  }
  // for macOS
  if (nasm)
    fprintf(fp, "global _%s\n", name);
  else
    fprintf(fp, ".global _%s\n", name);
  fprintf(fp, "_%s:\n", name);

  if (nasm)
    fprintf(fp, "global %s\n", name);
  else
    fprintf(fp, ".global %s\n", name);
  fprintf(fp, "%s:\n", name);
}

void add_gadget(FILE *fp, const char *format, ...) {
  va_list args;
  va_start(args, format);
#ifdef HOST_AARCH64
  fprintf(fp, ".dword ");
#elif defined(HOST_AMD64)
  if (nasm)
    fprintf(fp, "dq ");
  else
    fprintf(fp, ".dc.a ");
#elif defined(__powerpc__)
  fprintf(fp, ".dc.a ");
#elif defined(__loongarch__)
  fprintf(fp, ".dc.a ");
#endif
  vfprintf(fp, format, args);
  fprintf(fp, "\n");
  va_end(args);
}

// emit nops in nasm syntax
void emit_nasm_nops(FILE *fp, int repeat) {
  // handle overflow
  for (int i = 0; i < (repeat / 1000000); i++) {
    fprintf(fp, "\t%%rep 1000000\n");
    fprintf(fp, "\tnop\n");
    fprintf(fp, "\t%%endrep\n");
  }
  fprintf(fp, "\t%%rep %d\n", repeat % 1000000);
  fprintf(fp, "\tnop\n");
  fprintf(fp, "\t%%endrep\n");
}

void emit_multibyte_nops(FILE *fp, int length) {
  std::vector<std::vector<uint8_t>> encodings = {
      {0x90},
      {0x66, 0x90},
      {0x0F, 0x1F, 0x00},
      {0x0F, 0x1F, 0x40, 0x00},
      {0x0F, 0x1F, 0x44, 0x00, 0x00},
      {0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00},
      {0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00},
      {0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x66, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x66, 0x66, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x66, 0x66, 0x66, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00,
       0x00},
      {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00,
       0x00, 0x00},
      {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00,
       0x00, 0x00, 0x00},
  };
  assert(length >= 1 && length <= 15);
  for (auto byte : encodings[length - 1]) {
    fprintf(fp, "\t.byte 0x%x\n", byte);
  }
}

void arm64_la(FILE *fp, int reg, const char *format, ...) {
  va_list args;
  va_list tmp;
  va_start(args, format);
  // https://stackoverflow.com/a/65354324/2148614

#ifdef __linux__
  va_copy(tmp, args);
  fprintf(fp, "\tadrp x%d, ", reg);
  vfprintf(fp, format, tmp);
  fprintf(fp, "\n");

  fprintf(fp, "\tadd x%d, x%d, #:lo12:", reg, reg);
  vfprintf(fp, format, args);
  fprintf(fp, "\n");
#else
  // macOS
  va_copy(tmp, args);
  fprintf(fp, "\t adrp x%d, ", reg);
  vfprintf(fp, format, tmp);
  fprintf(fp, "@PAGE\n");

  fprintf(fp, "\t add x%d, x%d, ", reg, reg);
  vfprintf(fp, format, tmp);
  fprintf(fp, "@PAGEOFF\n");
#endif

#ifdef __linux__
#else
  // macOS
#endif

  va_end(args);
}

// adapted from: https://stackoverflow.com/a/45128487/2148614
typedef struct {
  uint64_t pfn : 55;
  unsigned int soft_dirty : 1;
  unsigned int file_page : 1;
  unsigned int swapped : 1;
  unsigned int present : 1;
} PagemapEntry;

/* Parse the pagemap entry for the given virtual address.
 *
 * @param[out] entry      the parsed entry
 * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
 * @param[in]  vaddr      virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr) {
  size_t nread;
  ssize_t ret;
  uint64_t data;
  uintptr_t vpn;

  vpn = vaddr / sysconf(_SC_PAGE_SIZE);
  nread = 0;
  while (nread < sizeof(data)) {
    ret = pread(pagemap_fd, ((uint8_t *)&data) + nread, sizeof(data) - nread,
                vpn * sizeof(data) + nread);
    nread += ret;
    if (ret <= 0) {
      return 1;
    }
  }
  entry->pfn = data & (((uint64_t)1 << 55) - 1);
  entry->soft_dirty = (data >> 55) & 1;
  entry->file_page = (data >> 61) & 1;
  entry->swapped = (data >> 62) & 1;
  entry->present = (data >> 63) & 1;
  return 0;
}

/* Convert the given virtual address to physical using /proc/self/pagemap.
 *
 * @param[out] paddr physical address
 * @param[in] vaddr virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int virt_to_phys_user(uintptr_t *paddr, uintptr_t vaddr) {
  int pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
  if (pagemap_fd < 0) {
    return 1;
  }
  PagemapEntry entry;
  if (pagemap_get_entry(&entry, pagemap_fd, vaddr)) {
    close(pagemap_fd);
    return 1;
  }
  close(pagemap_fd);
  if (entry.present == 0) {
    return 1;
  }
  *paddr =
      (entry.pfn * sysconf(_SC_PAGE_SIZE)) + (vaddr % sysconf(_SC_PAGE_SIZE));
  return 0;
}