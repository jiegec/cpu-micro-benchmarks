// taken from
// https://github.com/clamchowder/Microbenchmarks/blob/master/CoherencyLatency/PThreadsCoherencyLatency.c
// Originally licensed under Apache License 2.0
// Copyright clamchowder
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define ITERATIONS 10000000;

// kidding right?
#define gettid() syscall(SYS_gettid)

typedef struct LatencyThreadData {
  uint64_t start;
  uint64_t iterations;
  uint64_t *target;
  unsigned int processorIndex;
} LatencyData;

void *LatencyTestThread(void *param);
float RunTest(unsigned int processor1, unsigned int processor2, uint64_t iter);
uint64_t *bouncy;
uint64_t *bouncyBase;

int main(int argc, char *argv[]) {
  float **latencies;
  int numProcs, offsets = 1;
  uint64_t iter = ITERATIONS;

  numProcs = get_nprocs();
  fprintf(stderr, "Number of CPUs: %u\n", numProcs);

  if (0 != posix_memalign((void **)(&bouncyBase), 4096, 4096)) {
    fprintf(stderr, "Could not allocate aligned mem\n");
    return 0;
  }

  for (int argIdx = 1; argIdx < argc; argIdx++) {
    if (*(argv[argIdx]) == '-') {
      char *arg = argv[argIdx] + 1;
      if (strncmp(arg, "iterations", 10) == 0) {
        argIdx++;
        iter = atoi(argv[argIdx]);
        fprintf(stderr, "%lu iterations requested\n", iter);
      } else if (strncmp(arg, "bounce", 6) == 0) {
        fprintf(stderr, "Bouncy\n");
      } else if (strncmp(arg, "offset", 6) == 0) {
        argIdx++;
        offsets = atoi(argv[argIdx]);
        fprintf(stderr, "Offsets: %d\n", offsets);
      }
    }
  }

  latencies = (float **)malloc(sizeof(float *) * offsets);
  memset(latencies, 0, sizeof(float) * offsets);

  for (int offsetIdx = 0; offsetIdx < offsets; offsetIdx++) {
    latencies[offsetIdx] = (float *)malloc(sizeof(float) * numProcs * numProcs);
    float *latenciesPtr = latencies[offsetIdx];
    bouncy = (uint64_t *)((char *)bouncyBase + offsetIdx * 64);

    for (int i = 0; i < numProcs; i++) {
      for (int j = 0; j < numProcs; j++) {
        latenciesPtr[j + i * numProcs] = i == j ? 0 : RunTest(i, j, iter);
      }
    }
  }

  FILE *fp = fopen("coherency_latency.csv", "w");
  assert(fp);
  fprintf(fp, "offset");
  for (int i = 0; i < numProcs; i++) {
    fprintf(fp, ",core%d", i);
  }
  fprintf(fp, "\n");

  for (int offsetIdx = 0; offsetIdx < offsets; offsetIdx++) {
    float *latenciesPtr = latencies[offsetIdx];
    printf("Cache line offset: %d\n", offsetIdx);
    for (int i = 0; i < numProcs; i++) {
      fprintf(fp, "%d,", offsetIdx);
      for (int j = 0; j < numProcs; j++) {
        if (j != 0) {
          printf(",");
          fprintf(fp, ",");
        }
        if (j == i) {
          printf("x");
          fprintf(fp, "x");
        }
        // to maintain consistency, divide by 2 (see justification in windows
        // version)
        else {
          printf("%f", latenciesPtr[j + i * numProcs] / 2);
          fprintf(fp, "%f", latenciesPtr[j + i * numProcs] / 2);
        }
      }
      fprintf(fp, "\n");
      printf("\n");
    }

    free(latenciesPtr);
  }

  free(latencies);
  free(bouncyBase);
  return 0;
}

// run test and gather timing data using the specified thread function
float TimeThreads(unsigned int proc1, unsigned int proc2, uint64_t iter,
                  LatencyData *lat1, LatencyData *lat2,
                  void *(*threadFunc)(void *)) {
  struct timeval startTv, endTv;
  struct timezone startTz, endTz;
  pthread_t testThreads[2];
  int t1rc, t2rc;
  void *res1, *res2;

  gettimeofday(&startTv, &startTz);
  t1rc = pthread_create(&testThreads[0], NULL, threadFunc, (void *)lat1);
  t2rc = pthread_create(&testThreads[1], NULL, threadFunc, (void *)lat2);
  if (t1rc != 0 || t2rc != 0) {
    fprintf(stderr, "Could not create threads\n");
    return 0;
  }

  pthread_join(testThreads[0], &res1);
  pthread_join(testThreads[1], &res2);
  gettimeofday(&endTv, &endTz);

  uint64_t time_diff_ms = 1000 * (endTv.tv_sec - startTv.tv_sec) +
                          ((endTv.tv_usec - startTv.tv_usec) / 1000);
  float latency = 1e6 * (float)time_diff_ms / (float)iter;
  return latency;
}

// test latency between two logical CPUs
float RunTest(unsigned int processor1, unsigned int processor2, uint64_t iter) {
  LatencyData lat1, lat2;
  float latency;

  *bouncy = 0;
  lat1.iterations = iter;
  lat1.start = 1;
  lat1.target = bouncy;
  lat1.processorIndex = processor1;
  lat2.iterations = iter;
  lat2.start = 2;
  lat2.target = bouncy;
  lat2.processorIndex = processor2;
  latency = TimeThreads(processor1, processor2, iter, &lat1, &lat2,
                        LatencyTestThread);
  fprintf(stderr, "%d to %d: %f ns\n", processor1, processor2, latency);
  return latency;
}

void *LatencyTestThread(void *param) {
  LatencyData *latencyData = (LatencyData *)param;
  cpu_set_t cpuset;
  uint64_t current = latencyData->start;

  CPU_ZERO(&cpuset);
  CPU_SET(latencyData->processorIndex, &cpuset);
  sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpuset);
  // fprintf(stderr, "thread %ld set affinity %d\n", gettid(),
  // latencyData->processorIndex);

  while (current <= 2 * latencyData->iterations) {
    if (__sync_bool_compare_and_swap(latencyData->target, current - 1, current))
      current += 2;
  }

  pthread_exit(NULL);
}
