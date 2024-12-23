#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern void itlb_size(FILE *fp);
extern bool avoid_hugepage_merging;
extern int stride;
extern int fake_page_size;
int main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "hs:f:")) != -1) {
    switch (opt) {
    case 'h':
      avoid_hugepage_merging = true;
      break;
    case 's':
      sscanf(optarg, "%d", &stride);
      break;
    case 'f':
      sscanf(optarg, "%d", &fake_page_size);
      break;
    default:
      fprintf(stderr, "Usage: %s [-h] [-s stride] [-f page_size]\n", argv[0]);
      fprintf(stderr, "\t-h: avoid huge page merging\n");
      fprintf(stderr, "\t-s stride: set branch address stride\n");
      fprintf(stderr, "\t-f page_size: fake page size\n");
      exit(EXIT_FAILURE);
    }
  }

  FILE *fp = fopen("itlb_size.csv", "w");
  assert(fp);
  itlb_size(fp);
  printf("Results are written to itlb_size.csv\n");
  return 0;
}
