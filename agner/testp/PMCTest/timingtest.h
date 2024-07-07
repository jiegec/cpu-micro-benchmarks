/***************************  timingtest.h  ****************************
* Author:        Agner Fog
* Date created:  2014-04-15
* Last modified: 2014-04-15
* Project:       define functions for timing purposes etc.
* Description:
*
******************************************************************************/

#pragma once
#include <stdint.h>

#if defined(__WINDOWS__) || defined(_WIN32) || defined(_WIN64) 
// System-specific definitions for Windows

#if 1    // if intrin.h has __cpuid, __rdtsc and __readpmc

#include <intrin.h>

static inline void cpuid_ (int32_t output[4], int32_t functionnumber) {	
    __cpuid(output, functionnumber);
}

// serialize CPU by cpuid function 0
static inline void serialize () {
    int dummy[4];
    cpuid_(dummy, 0);
    // Prevent the compiler from optimizing away the whole Serialize function:
    volatile int DontSkip = dummy[0];
}

// read time stamp counter
static inline int64_t readtsc() {
    return __rdtsc();
}

// read performance monitor counter
static inline int64_t readpmc(int32_t nPerfCtr) {
    return __readpmc(nPerfCtr);
}


#else // intrin.h missing. use inline assembly

// inline MASM syntax

static inline void cpuid_ (int32_t output[4], int32_t functionnumber) {	
    __asm {
        mov eax, functionnumber;
        cpuid;
        mov esi, output;
        mov [esi],    eax;
        mov [esi+4],  ebx;
        mov [esi+8],  ecx;
        mov [esi+12], edx;
    }
}

static inline void serialize () {
    __asm {
        xor eax, eax;
        cpuid;
    }
}

// get time stamp counter
#pragma warning(disable:4035)
static inline uint64_t readtsc() {
    // read performance monitor counter number nPerfCtr
    __asm {
        rdtsc
    }
}

static inline uint64_t readpmc(int32_t nPerfCtr) {
    // read performance monitor counter number nPerfCtr
    __asm {
        mov ecx, nPerfCtr
            rdpmc
    }
}
#pragma warning(default:4035)

#endif


#elif defined(__unix__) || defined(__linux__)
// System-specific definitions for Linux

#include <cpuid.h>

static inline void cpuid_ (int32_t output[4], int32_t functionnumber) {	
    __get_cpuid(functionnumber, (uint32_t*)output, (uint32_t*)(output+1), (uint32_t*)(output+2), (uint32_t*)(output+3));
}

static inline void serialize () {
    __asm __volatile__ ("cpuid" : : "a"(0) : "ebx", "ecx", "edx" );  // serialize
}

// read time stamp counter
static inline uint64_t readtsc() {
    uint32_t lo, hi;
    __asm __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi) : : );
    return lo | (uint64_t)hi << 32;
}

// read performance monitor counter
static inline uint64_t readpmc(int32_t n) {
    uint32_t lo, hi;
    __asm __volatile__ ("rdpmc" : "=a"(lo), "=d"(hi) : "c"(n) : );
    return lo | (uint64_t)hi << 32;
}


#else  // not Windows or Unix

#error Unknown platform

#endif
