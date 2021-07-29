#include "include/utils.h"
#include <random>
#include <sys/time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
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
#elif defined(__APPLE__)
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

uint64_t get_time_ns() {
  struct timeval tv = {};
  gettimeofday(&tv, nullptr);
  return (uint64_t)tv.tv_sec * 1000000000 + (uint64_t)tv.tv_usec * 1000;
}

char **generate_random_pointer_chasing(size_t size) {
  int count = size / sizeof(char *);
  char **buffer = new char *[count];
  int *index = new int[count];

  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());

  // init index and shuffle
  for (int i = 0; i < count; i++) {
    index[i] = i;
  }
  for (int i = 1; i < count; i++) {
    std::uniform_int_distribution<int> distr(0, i - 1);
    int j = distr(generator);
    int temp = index[i];
    index[i] = index[j];
    index[j] = temp;
  }

  // init circular list
  for (int i = 0; i < count - 1; i++) {
    buffer[index[i]] = (char *)&buffer[index[i + 1]];
  }
  buffer[index[count - 1]] = (char *)&buffer[index[0]];

  delete[] index;

  return buffer;
}