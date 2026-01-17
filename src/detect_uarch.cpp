#include "include/uarch.h"
#include <cassert>
#include <stdio.h>

int main() {
  enum uarch uarch = get_uarch();
  switch (uarch) {
  case firestorm:
    printf("-DAPPLE_SILICON\n");
    printf("-DAPPLE_PCORE\n");
    printf("-DAPPLE_M1\n");
    printf("-DAPPLE_M1_FIRESTORM\n");
    break;
  case icestorm:
    printf("-DAPPLE_SILICON\n");
    printf("-DAPPLE_M1\n");
    printf("-DAPPLE_M1_ICESTORM\n");
    break;
  case avalanche:
    printf("-DAPPLE_SILICON\n");
    printf("-DAPPLE_PCORE\n");
    printf("-DAPPLE_M2\n");
    printf("-DAPPLE_M2_AVALANCHE\n");
    break;
  case blizzard:
    printf("-DAPPLE_SILICON\n");
    printf("-DAPPLE_M2\n");
    printf("-DAPPLE_M2_BLIZZARD\n");
    break;
  case m4_pcore:
    printf("-DAPPLE_SILICON\n");
    printf("-DAPPLE_PCORE\n");
    printf("-DAPPLE_M4\n");
    printf("-DAPPLE_M4_PCORE\n");
    break;
  case m4_ecore:
    printf("-DAPPLE_SILICON\n");
    printf("-DAPPLE_M4\n");
    printf("-DAPPLE_M4_ECORE\n");
    break;
  case oryon:
    printf("-DQUALCOMM_ORYON\n");
    break;
  case cortex_a78:
    printf("-DARM_CORTEX_A78\n");
    break;
  case cortex_a77:
    printf("-DARM_CORTEX_A77\n");
    break;
  case cortex_x1:
    printf("-DARM_CORTEX_X1\n");
    break;
  case neoverse_n1:
    printf("-DNO_FJCVTZS\n");
    printf("-DARM_NEOVERSE_N1\n");
    break;
  case neoverse_v1:
    printf("-DARM_NEOVERSE_V1\n");
    break;
  case neoverse_n2:
    printf("-DARM_NEOVERSE_N2\n");
    break;
  case neoverse_v2:
    printf("-DARM_NEOVERSE_V2\n");
    break;
  case tsv110:
    printf("-DHISILICON_TSV110\n");
    break;
  case unknown_arm64:
    break;
  case granite_rapids:
    printf("-DINTEL\n");
    printf("-DINTEL_GRANITE_RAPIDS\n");
    break;
  case golden_cove:
    printf("-DINTEL\n");
    printf("-DINTEL_AHYBRID\n");
    break;
  case gracemont:
    printf("-DINTEL\n");
    printf("-DINTEL_AHYBRID\n");
    break;
  case sunny_cove:
    printf("-DINTEL\n");
    printf("-DINTEL_ICELAKE_SERVER\n");
    break;
  case skylake:
    printf("-DINTEL\n");
    printf("-DINTEL_SKYLAKE_SERVER\n");
    break;
  case broadwell:
    printf("-DINTEL\n");
    printf("-DINTEL_BROADWELL\n");
    break;
  case zen1:
    printf("-DAMD\n");
    printf("-DAMD_ZEN1\n");
    break;
  case zen2:
    printf("-DAMD\n");
    printf("-DAMD_ZEN2\n");
    break;
  case zen3:
    printf("-DAMD\n");
    printf("-DAMD_ZEN3\n");
    break;
  case zen4:
    printf("-DAMD\n");
    printf("-DAMD_ZEN4\n");
    break;
  case zen5:
    printf("-DAMD\n");
    printf("-DAMD_ZEN5\n");
    break;
  case unknown_amd64:
    break;
  case la464:
    printf("-DLA464\n");
  case unknown_loongarch64:
    break;
  case power8:
    printf("-DPOWER8\n");
  case power9:
    printf("-DPOWER9\n");
  case unknown_ppc64le:
    break;
  default:
    assert(false);
  }
  return 0;
}
