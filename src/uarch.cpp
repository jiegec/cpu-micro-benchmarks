#include "include/uarch.h"
#include <assert.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

static int bind_core = 0;
static enum uarch current_uarch = unknown;

struct uarch_mapping {
  const char *name;
  enum uarch uarch;
  int bind_core;
};

enum uarch get_uarch_inner() {
  std::vector<struct uarch_mapping> mapping = {
      {"Gracemont", gracemont, 24},
      {"Icestorm", icestorm, 0},
  };
  const char *override = getenv("UARCH_OVERRIDE");
  if (override) {
    for (auto entry : mapping) {
      if (strcasecmp(override, entry.name) == 0) {
        printf("Uarch overridden with %s\n", entry.name);
        bind_core = entry.bind_core;
        return entry.uarch;
      }
    }
  }

  // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
  std::ifstream t("/sys/devices/system/node/node0/cpu0/of_node/compatible");
  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string content = buffer.str();
  if (content.find("apple,icestorm") != std::string::npos) {
    fprintf(stderr, "Apple M1 detected\n");

    fprintf(stderr, "Configured for Apple M1 Firestorm\n");
    bind_core = 4;
    return firestorm;
  }
  if (content.find("qcom,oryon") != std::string::npos) {
    fprintf(stderr, "Qualcomm Oryon detected\n");
    return oryon;
  }

  t.close();
  t.open("/sys/firmware/devicetree/base/compatible");

  buffer.clear();
  buffer << t.rdbuf();
  content = buffer.str();
  if (content.find("qcom,sc8280xp") != std::string::npos) {
    fprintf(stderr, "Qualcomm 8cx Gen 3 detected\n");

    fprintf(stderr, "Configured for Cortex-X1\n");
    bind_core = 4;
    return cortex_x1;
  }

  t.close();
  t.open("/proc/cpuinfo");

  std::string line;

  // x86
  int family = 0;
  int model = 0;
  // arm64
  int implementer = 0;
  int part = 0;
  bool sve = false;
  bool avx512f = false;
  bool avx2 = false;

  while (std::getline(t, line)) {
    size_t pos = line.find(':');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      key.erase(key.find_last_not_of(" \t") + 1);
      std::string value = line.substr(pos + 1);
      if (key == "cpu family") {
        family = std::stoi(value);
      } else if (key == "model") {
        model = std::stoi(value);
      } else if (key == "CPU implementer") {
        implementer = std::stoi(value, nullptr, 16);
      } else if (key == "CPU part") {
        part = std::stoi(value, nullptr, 16);
      } else if (key == "Model Name" && value == " Loongson-3C5000") {
        return la464;
      } else if (key == "flags") {
        if (value.find("avx512f") != std::string::npos && !avx512f) {
          avx512f = true;
        }
        if (value.find("avx2") != std::string::npos && !avx2) {
          avx2 = true;
        }
      } else if (key == "Features") {
        if (value.find("sve") != std::string::npos && !sve) {
          sve = true;
        }
      }
    }
  }
  fprintf(stderr, "Found CPU family %d, model %d, implementer %d, part %d\n",
          family, model, implementer, part);

  if (avx2) {
    fprintf(stdout, "AVX2 detected\n");
  }
  if (avx512f) {
    fprintf(stdout, "AVX512F detected\n");
  }
  if (sve) {
    fprintf(stdout, "SVE detected\n");
  }

  if (family == 6 && model == 183) {
    fprintf(stderr, "Intel Raptor Lake detected\n");
    fprintf(stderr, "Configured for Golden Cove\n");
    return golden_cove;
  } else if (family == 6 && model == 151) {
    fprintf(stderr, "Intel Alder Lake detected\n");
    return golden_cove;
  } else if (family == 6 && model == 106) {
    fprintf(stderr, "Intel Ice Lake detected\n");
    return sunny_cove;
  } else if (family == 6 && model == 85) {
    fprintf(stderr, "Intel Skylake detected\n");
    return skylake;
  } else if (family == 6 && model == 79) {
    fprintf(stderr, "Intel Broadwell detected\n");
    return broadwell;
    // https://en.wikichip.org/wiki/amd/cpuid
  } else if (family == 6 && model == 142) {
    fprintf(stderr, "Intel Whiskylake detected\n");
    return skylake;
  } else if (family == 23 && model == 1) {
    fprintf(stderr, "AMD Zen 1 detected\n");
    return zen1;
  } else if (family == 23 && model == 49) {
    fprintf(stderr, "AMD Zen 2 detected\n");
    return zen2;
  } else if (family == 25 && (model == 1 || model == 33)) {
    fprintf(stderr, "AMD Zen 3 detected\n");
    return zen3;
  } else if (family == 25 && (model == 17 || model == 97)) {
    fprintf(stderr, "AMD Zen 4 detected\n");
    return zen4;
  } else if (family == 26 && model == 68) {
    fprintf(stderr, "AMD Zen 5 detected\n");
    return zen5;
    // https://github.com/util-linux/util-linux/blob/master/sys-utils/lscpu-arm.c
  } else if (implementer == 0x41 && part == 0xd0c) {
    fprintf(stderr, "ARM Neoverse N1 detected\n");
    return neoverse_n1;
  } else if (implementer == 0x41 && part == 0xd0d) {
    fprintf(stderr, "ARM Cortex A77 detected\n");
    return cortex_a77;
  } else if (implementer == 0x41 && part == 0xd41) {
    fprintf(stderr, "ARM Cortex A78 detected\n");
    return cortex_a78;
  } else if (implementer == 0x41 && part == 0xd05) {
    fprintf(stderr, "ARM Cortex A55 detected\n");
    return cortex_a55;
  } else if (implementer == 0x41 && part == 0xd03) {
    fprintf(stderr, "ARM Cortex A75 detected\n");
    return cortex_a53;
  } else if (implementer == 0x41 && part == 0xd09) {
    fprintf(stderr, "ARM Cortex A76 detected\n");
    return cortex_a73;
  } else if (implementer == 0x41 && part == 0xd40) {
    fprintf(stderr, "ARM Neoverse V1 detected\n");
    return neoverse_v1;
  } else if (implementer == 0x41 && part == 0xd49) {
    fprintf(stderr, "ARM Neoverse N2 detected\n");
    return neoverse_n2;
  } else if (implementer == 0x41 && part == 0xd4f) {
    fprintf(stderr, "ARM Neoverse V2 detected\n");
    return neoverse_v2;
  } else if (implementer == 0x48 && part == 0xd01) {
    fprintf(stderr, "Hisilicon TSV110 detected\n");
    return tsv110;
  }

#ifdef __APPLE__
  FILE *fp = popen("sysctl -nx hw.cpufamily", "r");
  char buf[128];
  fgets(buf, sizeof(buf), fp);
  if (strcmp(buf, "0x1b588bb3\n") == 0) {
    fprintf(stderr, "Apple M1 detected\n");

    fprintf(stderr, "Configured for Apple M1 Firestorm\n");
    return firestorm;
  } else if (strcmp(buf, "0xda33d83d\n") == 0) {
    fprintf(stderr, "Apple M2 detected\n");

    fprintf(stderr, "Configured for Apple M2 Avalanche\n");
    return avalanche;
  }
#endif

#ifdef HOST_AARCH64
  return unknown_arm64;
#elif defined(HOST_AMD64)
  return unknown_amd64;
#elif defined(HOST_LOONGARCH64)
  return unknown_loongarch64;
#endif
  return unknown;
}

enum uarch get_uarch() {
  if (current_uarch == unknown) {
    current_uarch = get_uarch_inner();
    assert(current_uarch != unknown);
    // handle bind_core override
    const char *env = getenv("BIND_CORE_OVERRIDE");
    if (env) {
      int core = -1;
      if (sscanf(env, "%d", &core) > 0) {
        fprintf(stderr, "Core binding overrided from %d to %d\n", bind_core,
                core);
        bind_core = core;
      }
    }
  }
  return current_uarch;
}

// which core to bind
int get_bind_core() {
  if (current_uarch == unknown) {
    get_uarch();
  }
  return bind_core;
}
