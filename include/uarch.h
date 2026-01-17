#ifndef __UARCH_H__
#define __UARCH_H__

enum uarch {
  // special
  unknown,

  // arm64
  // apple
  // m1
  firestorm,
  icestorm,
  // m2
  avalanche,
  blizzard,
  // m4
  m4_pcore,
  m4_ecore,
  // qualcomm
  oryon,
  // arm
  cortex_a77,
  cortex_a78,
  cortex_x1,
  neoverse_n1,
  neoverse_v1,
  neoverse_n2,
  neoverse_v2,
  // hisilicon
  tsv110,

  unknown_arm64,
  arm64_begin = firestorm,
  arm64_end = unknown_arm64,

  // ppc64le
  power8,
  power9,

  unknown_ppc64le,
  ppc64le_begin = power8,
  ppc64le_end = unknown_ppc64le,

  // loongarch
  la464,
  la664,
  unknown_loongarch64,
  loongarch64_begin = la464,
  loongarch64_end = unknown_loongarch64,

  // intel
  granite_rapids,
  golden_cove,
  gracemont,
  sunny_cove,
  skylake,
  broadwell,
  // amd
  zen1,
  zen2,
  zen3,
  zen4,
  zen5,

  unknown_amd64,

  // valid range
  all_begin = firestorm,
  all_end = unknown_amd64,
};

// detect uarch
enum uarch get_uarch();
// which core to bind
int get_bind_core();

#endif
